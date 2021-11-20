#include "INeuralNet.hpp"
#include "filesystem.hpp"
#include "hcore.hpp"
#include "hsocket.hpp"
#include "module.hpp"
#include "settings.hpp"

bool running = true;
image_t detectionImage;
int commandBuffer[0x20];
INeuralNet *detector;

HCore::HCore *mainCore;

class DetectorModule : public Module {
  public:
    MFunc<INeuralNet *> m_detectorCreate;

    void loadCreateDetector() {
        mainCore->log(LOG_INFO, "Loading function");
#ifdef _WIN32
        m_detectorCreate["createDetector"] = (moduleFunc<INeuralNet *>)GetProcAddress(m_library, "createDetector");
#else
        m_detectorCreate["createDetector"] = (moduleFunc<INeuralNet *>)dlsym(m_library, "createDetector");
#endif
        if (m_detectorCreate["createDetector"] == NULL) {
            mainCore->log(LOG_ERROR, "Function null");
        } else {
            mainCore->log(LOG_INFO, "Function loaded");
        }
    }
};

void allocateBuffer(int width, int height) {
    int size = width * height * 3;
    mainCore->log(LOG_INFO, "allocating %d", size);
    detectionImage.data = new float[width * height * 3];
    detectionImage.bdata = new unsigned char[width * height * 3];
}

void mainLoop() {

    OS::socket_receive(commandBuffer, 0x20 * 4);
    detectionImage.w = commandBuffer[0];
    detectionImage.h = commandBuffer[1];
    detectionImage.detectionX = commandBuffer[2];
    detectionImage.detectionY = commandBuffer[3];
    detectionImage.detectionW = commandBuffer[4];
    detectionImage.detectionH = commandBuffer[5];
    detectionImage.c = 3;
    float thresh = (float)commandBuffer[8] / 100.0;

    OS::socket_receive(detectionImage.bdata, detectionImage.w * detectionImage.h * detectionImage.c);
    for (int i = 0; i < detectionImage.w * detectionImage.h * detectionImage.c; i++) {
        detectionImage.data[i] = (float)detectionImage.bdata[i] / 255.0f;
    }
    std::vector<bbox_t> dets = detector->detect(detectionImage, thresh);
    dets.resize(24);
    commandBuffer[0] = 24;

    OS::socket_send(dets.data(), sizeof(bbox_t) * 24);
}

int main(int argc, char **argv) {
    printf("Hayha detection daemon\nSet your ip and change mode to "
           "remote"
           " in Settings.json to use\n");
    /*
    if(argc < 4){
        fprintf(stderr,"\n\nUsage: %s [lib file] [config file] [weight
    file]\n\n",argv[0]); return 1;
    }
    */
    OS::initPaths();
    Settings::loadSettings(OS::cfgPath + "Settings.json");
    mainCore = HCore::init(2, 1);

    allocateBuffer(2048, 2048);

    DetectorModule m_module;
    std::string libname = OS::libPath + OS::libPrefix + Settings::net.libFile + OS::libSuffix;
    m_module.loadLibrary(libname.c_str());
    m_module.loadCreateDetector();
    detector = m_module.m_detectorCreate["createDetector"](Settings::net.configFile, Settings::net.weightFile, Settings::net.gpuIndex);

    OS::socket_init(OS::SERVER);
    OS::socket_listen(1939);
    while (running) {
        int client = OS::socket_accept();
        mainLoop();
        OS::socket_close();
    }
    OS::socket_close();
    return 0;
}