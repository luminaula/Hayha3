#include <fstream>
#include <iostream>
#include <string.h>
#include <string>

#include "filesystem.hpp"
#include "hcore.hpp"
#include "json.hpp"
#include "settings.hpp"

using json = nlohmann::json;

#include <QApplication>
#include <QDesktopWidget>
#include <QRect>

namespace Settings {
CoreSettings_t core;
CaptureSettings_t capture;
DetectionSettings_t detection;
DisplaySettings_t display;
NetSettings_t net;
TargeterSettings_t target;
OverlaySettings_t overlay;
MouseSettings_t mouse;
KeyboardSettings_t keyboard;
Area_t screen;
Scripts_t scripts;
TrainSettings_t train;

void processSettings() {
    int x, y;

    QRect rec = QApplication::desktop()->screenGeometry();
    screen.h = rec.height();
    screen.w = rec.width();

    if (overlay.enabled) {
        capture.DXGI = false;
        detection.useNetResolution = true;
        capture.captureDetectionArea = true;
    }

    if (detection.useNetResolution) {
        detection.w = net.w * detection.netRatio;
        detection.h = net.h * detection.netRatio;
    }
    if (detection.center) {
        x = capture.x + capture.w / 2;
        y = capture.y + capture.h / 2;
        detection.x = x - detection.w / 2;
        detection.y = y - detection.h / 2;
    }

    if (capture.captureDetectionArea) {
        capture.x = detection.x;
        capture.y = detection.y;
        capture.h = detection.h;
        capture.w = detection.w;
    }
}

bool loadSettings(std::string filename) {

    // printf("Loading settings from %s\n",filename.c_str());

    std::ifstream i(filename);
    json setit;
    i >> setit;

    core.threadCount = setit["Core"]["Threads"].get<int>();
    core.logLevel = setit["Core"]["LogLevel"].get<int>();

    sprintf(net.libFile, "%s", setit["Net"]["Library"].get<std::string>().c_str());
    sprintf(net.profile, "%s", setit["Net"]["Profile"].get<std::string>().c_str());

    net.w = setit["Net"]["Width"].get<int>() * 32;
    net.h = setit["Net"]["Height"].get<int>() * 32;
    net.gpuIndex = setit["Net"]["GpuIndex"].get<int>();

    sprintf(net.dataFile, "%s/%s/net.data", OS::profilesPath.c_str(), net.profile);
    sprintf(net.configFile, "%s/%s/net.cfg", OS::profilesPath.c_str(), net.profile);
    sprintf(net.weightFile, "%s/%s/net.weight", OS::profilesPath.c_str(), net.profile);
    sprintf(net.trainFile, "%s/%s/train.cfg", OS::profilesPath.c_str(), net.profile);
    sprintf(net.classifyConfigFile, "%s/%s/class.cfg", OS::profilesPath.c_str(), net.profile);
    sprintf(net.classifyWeightFile, "%s/%s/class.weight", OS::profilesPath.c_str(), net.profile);

    sprintf(target.idFile, "%s/%s/targets.json", OS::profilesPath.c_str(), net.profile);

    net.precision = setit["Net"]["Precision"].get<std::string>();

    capture.rate = setit["Capture"]["Rate"].get<int>();
    capture.x = setit["Capture"]["X"].get<int>();
    capture.y = setit["Capture"]["Y"].get<int>();
    capture.w = setit["Capture"]["Width"].get<int>();
    capture.h = setit["Capture"]["Height"].get<int>();
    capture.captureDetectionArea = setit["Capture"]["CaptureDetectionArea"].get<bool>();
    capture.async = setit["Capture"]["Async"].get<bool>();
    capture.DXGI = setit["Capture"]["DXGI"].get<bool>();
    capture.format = setit["Capture"]["Format"].get<std::string>();

#ifdef linux
    capture.target = setit["Capture"]["X11Window"].get<std::string>();
#else
    capture.target = setit["Capture"]["Window"].get<std::string>();
#endif
    detection.rate = setit["Detect"]["Rate"].get<int>();
    detection.x = setit["Detect"]["X"].get<int>();
    detection.y = setit["Detect"]["Y"].get<int>();
    detection.w = setit["Detect"]["Width"].get<int>();
    detection.h = setit["Detect"]["Height"].get<int>();
    detection.thresh = setit["Detect"]["Thresh"].get<int>();
    detection.center = setit["Detect"]["Center"].get<bool>();
    detection.useNetResolution = setit["Detect"]["UseNetResolution"].get<bool>();
    detection.netRatio = setit["Detect"]["NetRatio"].get<float>();
    detection.aroundMouse = setit["Detect"]["AroundMouse"].get<bool>();

    bool remote = setit["Detect"]["Remote"].get<bool>();
    if (remote) {
        detection.mode = REMOTE;
    } else {
        detection.mode = LOCAL;
    }
    sprintf(detection.IP, "%s", setit["Detect"]["IP"].get<std::string>().c_str());

    sprintf(train.libfile, "%s", setit["Train"]["Library"].get<std::string>().c_str());
    for (auto &gpu : setit["Train"]["GPUs"]) {
        train.gpus.push_back(gpu.get<int>());
    }

    // display.drawDetections = setit["Display"]["DrawDetections"].get<bool>();
    display.rate = setit["Display"]["Rate"].get<int>();
    display.w = setit["Display"]["Width"].get<int>();
    display.h = setit["Display"]["Height"].get<int>();
    display.enabled = setit["Display"]["Enabled"].get<bool>();

    overlay.enabled = setit["Overlay"]["Enabled"].get<bool>();
    overlay.rate = setit["Overlay"]["Rate"].get<int>();
    overlay.showStats = setit["Overlay"]["ShowStats"].get<bool>();
    overlay.showArea = setit["Overlay"]["ShowArea"].get<bool>();

    sprintf(scripts.entry, "%s%s", OS::scriptPath.c_str(), setit["Scripts"]["Entry"].get<std::string>().c_str());

    sprintf(scripts.weapon, "%sweapons/?.lua", OS::scriptPath.c_str());
    sprintf(scripts.util, "%sutil/?.lua", OS::scriptPath.c_str());
    processSettings();

    return true;
}

} // namespace Settings
