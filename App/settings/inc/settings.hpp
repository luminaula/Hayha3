#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <string>
#include <vector>



struct Area_t {
    int x, y, h, w;
};

struct FloatArea_t {
    float x, y, w, h;
};

// TODO from c strings to c++ strings

namespace Settings {

struct CoreSettings_t {
    uint32_t threadCount;
    uint32_t logLevel;
};

struct NetSettings_t : Area_t {
    char libFile[0x800];
    char dataFile[0x800];
    char configFile[0x800];
    char weightFile[0x800];
    char trainFile[0x800];
    char classifyConfigFile[0x800];
    char classifyWeightFile[0x800];
    char profile[0x800];
    int gpuIndex;
    std::string precision;
};

struct TrainSettings_t {
    char libfile[0x800];
    char weightFile[0x800];
    char configFile[0x800];
    int iters;
    std::vector<int> gpus;
};

struct CaptureSettings_t : Area_t {
    bool captureDetectionArea;
    bool async;
    bool DXGI;
    int rate;
    std::string target;
    std::string format;
};

enum DetectionMode { LOCAL, REMOTE };

struct DetectionSettings_t : Area_t {
    char IP[0x800];
    DetectionMode mode;
    bool center;
    int thresh;
    bool useNetResolution;
    float netRatio;
    bool aroundMouse;
    int rate;
};

struct DisplaySettings_t : Area_t {
    bool enabled;
    int rate;
    bool drawDetections;
};

struct OverlaySettings_t {
    bool enabled;
    bool showStats;
    bool showCrosshair;
    bool showArea;
    int rate;
};

struct TargeterSettings_t {
    char idFile[0x800];
};

struct MouseSettings_t {
    char script[0x800];
    int rate;
};

struct KeyboardSettings_t {
    char script[0x800];
    int rate;
};

struct Scripts_t {
    char entry[0x800];
    char util[0x800];
    char weapon[0x800];
    char weaponName[0x800];
};

extern Scripts_t scripts;
extern CoreSettings_t core;
extern NetSettings_t net;
extern Area_t screen;
extern CaptureSettings_t capture;
extern DetectionSettings_t detection;
extern DisplaySettings_t display;
extern OverlaySettings_t overlay;
extern TargeterSettings_t target;
extern MouseSettings_t mouse;
extern KeyboardSettings_t keyboard;
extern TrainSettings_t train;

void processSettings();

bool loadSettings(std::string filename);
} // namespace Settings

#endif