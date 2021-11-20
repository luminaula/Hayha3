#ifndef HAPI_HPP
#define HAPI_HPP

#include "OScommon.hpp"
#include "hcore.hpp"
#include "keyboard.hpp"
#include "mouse.hpp"
#include "route.hpp"

#ifdef _WIN32
#include "wincommon.hpp"
#else
#include "X11common.hpp"
#include "X11keyboard.hpp"
#include "X11mouse.hpp"
#endif

namespace HAPI {

extern HCore::HCore *m_core;

extern std::atomic<bool> m_clicking;
extern timeStamp m_clickStart;
extern timeStamp m_clickStop;
extern bool m_center;

void init(HCore::HCore *core);

void decay();
void setDecay(float value);
void setIncrement(float value);
void setMouseRate(float value);
void setKeyboardRate(float value);
OS::PointerLocation pointerLocation();
void ignoreArea(float x0, float y0, float w, float h);
void clearIgnore();
void centerPointer(bool value);
int64_t time();

float getClickState();
void checkMouseClick();
int timeClicked();
void click(uint32_t clickTime);

void moveMouse(float x, float y, uint32_t usec);

void clearKeys();
void listenKey(uint32_t code);
bool checkKey(uint32_t code);
void strictKeyboard(bool val);

void setCaptureOffset(int x, int y, uint32_t utime);
void setCaptureCenter(int x, int y, uint32_t utime);

void setNetworkResolution(int width, int height);
void setDetectionNetRatio(float value);

void readRoute(std::string name);
void clearRoutes();
void setRoute(std::string name);
void startCurrentRoute();
void travelCurrentRoute(uint32_t utime);
Util::RoutePoint getCurrentLocation();
void startRoute(std::string name);
void travelRoute(std::string name, uint32_t utime);
Util::RoutePoint getLocation(std::string name);

std::string scriptsPath();

std::string getWeapon();
std::string getWeaponPath();
std::string getUtilPath();
void setWeapon(std::string name);

void log(const char *logEntry);

void sleep(uint32_t usec);

void refreshTrainData();
void train(uint32_t iters);

bool training();

} // namespace HAPI

#endif