#include "hdetector.hpp"
#include "buffer.hpp"
#include "capture.hpp"
#include "filesystem.hpp"
#include "hcore.hpp"
#include "hsocket.hpp"
#include "math.h"
#include "modules.hpp"
#include "mouse.hpp"
#include "workers.hpp"
#include <string.h>

HDetector::HDetector(HCore::HCore *core) : Threadable(Settings::detection.rate, "Detector", false, core) {

    yoloDetector = NULL;
    m_limit = false;

    //m_core->tPool->enqueuev(&HDetector::loadDetector, this);
}

void HDetector::loadDetector() {
    std::lock_guard<std::mutex> lock(m_loadMutex);
    if (yoloDetector)
        return;
    NetCreateInfo info;
    sprintf(info.config, "%s", Settings::net.configFile);
    sprintf(info.weight, "%s", Settings::net.weightFile);
    sprintf(info.folder, "%s/%s/", OS::profilesPath.c_str(), Settings::net.profile);
    sprintf(info.precision, "%s", Settings::net.precision.c_str());
    info.gpu = Settings::net.gpuIndex;
    info.height = Settings::net.h;
    info.width = Settings::net.w;

    if (!yoloDetector) {

        if (strcmp(Settings::net.libFile, "HNetCl") == 0) {
            m_core->log(LOG_ERROR, "OpenCl runtime has been removed due to it being too slow");
            // yoloDetector = new
            // ClDetector(Settings::net.configFile,Settings::net.weightFile,0);
            yoloDetector = new INeuralNet();

        }

        else if (strcmp(Settings::net.libFile, "DEBUG") == 0) {
            yoloDetector = new INeuralNet();
        } else {

            std::string libname = OS::libPath + OS::libPrefix + Settings::net.libFile + OS::libSuffix;

            if (!loadLibrary(libname.c_str())) {
                m_core->log(LOG_ERROR, "Detector library %s IS NOT loaded", libname.c_str());
                m_core->log(LOG_ERROR, "Make sure cuda is installed and your operating "
                                       "system can find cuda libraries");
                yoloDetector = new INeuralNet();
                return;
            }
            m_detectorCreate["createDetector"] = loadFunc<INeuralNet *>("createDetector");
            if (m_detectorCreate["createDetector"] == NULL) {
                m_core->log(LOG_ERROR, "Net create function was not found");
                yoloDetector = new INeuralNet();
                return;
            }
            m_core->log(LOG_INFO, "Loading network...");
            yoloDetector = m_detectorCreate["createDetector"](info);
            // yoloDetector->loadClassifier(Settings::net.classifyConfigFile,Settings::net.classifyWeightFile,1);
            if (yoloDetector) {
                m_core->log(LOG_INFO, "Network loaded succesfully");
            } else {
                m_core->log(LOG_ERROR, "Error loading Network");
                yoloDetector = new INeuralNet();
                return;
            }
        }
    }
}

void HDetector::detectRemote(image_t img) {
    if (yoloDetector) {
        delete yoloDetector;
    }
    OS::socket_connect(Settings::detection.IP, 1939);

    int commandBuffer[0x20];
    commandBuffer[0] = img.w;
    commandBuffer[1] = img.h;
    commandBuffer[2] = img.detectionX;
    commandBuffer[3] = img.detectionY;
    commandBuffer[4] = img.detectionW;
    commandBuffer[5] = img.detectionH;
    commandBuffer[6] = 0;
    commandBuffer[7] = 0;
    commandBuffer[8] = Settings::detection.thresh;

    OS::socket_send(commandBuffer, 0x20 * 4);

    bbox_t *boxies = new bbox_t[24];

    OS::socket_send(img.bdata, img.w * img.h * 3);
    OS::socket_receive(boxies, sizeof(bbox_t) * 24);
    m_detections.clear();
    for (int i = 0; i < 24; i++) {
        if (boxies[i].x != 0)
            m_detections.push_back(boxies[i]);
        else
            break;
    }

    OS::socket_close();

    delete boxies;
}

void HDetector::detectLocal(image_t img) {
    if (!yoloDetector) {
        loadDetector();
    }
    m_detections = yoloDetector->detect(img, (float)Settings::detection.thresh / 100.0);
    // m_core->log(LOG_INFO,"%d",yoloDetector->classify(img));
}

void HDetector::benchmark(int height, int width, int times) {
    image_t img;
    img.w = width;
    img.h = height;
    img.detectionH = height;
    img.detectionW = width;
    img.detectionX = 0;
    img.detectionY = 0;
    img.c = 3;
    img.bdata = new unsigned char[img.detectionH * img.detectionW * 4];
    img.data = new float[img.detectionH * img.detectionW * img.c];
    for (int i = 0; i < times; i++) {
        switch (Settings::detection.mode) {
        case Settings::LOCAL:
            detectLocal(img);
            break;
        case Settings::REMOTE:
            detectRemote(img);
            break;
        }
    }
    delete img.bdata;
    delete img.data;
}

void HDetector::benchmark() {
    m_core->log(LOG_INFO, "Begin benchmark");
    timeStamp startTime = getCurrentTimeMicro();
    int times = 100;
    for (int height = 96, width = 96; height <= 1024; height += 32, width += 32) {
        benchmark(height, width, 1);
        timeStamp benchStart = getCurrentTimeMicro();
        benchmark(height, width, times);
        timeStamp benchEnd = getCurrentTimeMicro();
        int32_t totalTime = getTimeDifference(benchEnd, benchStart);
        float avgTime = (float)totalTime / times;
        float fps = 1000000.0 / avgTime;
        m_core->log(LOG_INFO, "Benchmark %d times %dx%d total %fs avg %fms %ffps", times, width, height, (float)totalTime / 1000000.0,
                    avgTime / 1000.0, fps);
    }
    m_core->log(LOG_INFO, "End Benchmark");
}

void HDetector::work() {

    if (!Workers::capture->isRunning()) {
        // Capture a new frame if asynchronous capturing is not enabled
        Workers::capture->singleLoop();
    } else {
        // Wait for a fresh frame to finish
        conditionLock();
    }

    Framebuffer &fb = HBuffer::getFramebuffer(HBuffer::DETECT);

    if (fb.m_detected)
        return;

    fb.m_detectionsPrev = m_detections;

    // m_core->log(LOG_DEBUG,"Begin detection");
    fb.m_beginDetect = getCurrentTimeMicro();
    image_t img;
    img.w = fb.fWidth;
    img.h = fb.fHeight;
    img.detectionX = fb.detection.x;
    img.detectionY = fb.detection.y;
    img.detectionH = fb.detection.h;
    img.detectionW = fb.detection.w;
    img.c = 3;
    img.data = fb.fdata;
    img.bdata = fb.rdata;
    img.thresh = (float)Settings::detection.thresh / 100.0;
    switch (Settings::detection.mode) {
    case Settings::LOCAL:
        detectLocal(img);
        break;
    case Settings::REMOTE:
        detectRemote(img);
        break;
    }

    fb.m_endDetect = getCurrentTimeMicro();

    fb.m_detections = m_detections;
    fb.m_detected = true;

    HBuffer::finishDetect(fb.uid);

    /*
    if(!fb.m_detections.empty()){
        m_core->tPool->enqueuev([&fb](){
            Modules::tracker->m_void["setDimensions"](fb.capture.x,fb.capture.y,fb.capture.w,fb.capture.h);
            Modules::tracker->m_void["resetTracker"](fb.data,fb.m_detections);
        });
    }
    */

    Workers::statServer->pushStat("detect");
}
