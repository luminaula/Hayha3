#include "HApi.hpp"
#include "workers.hpp"

namespace HAPI {

HCore::HCore *m_core;

std::atomic<bool> m_clicking(false);
timeStamp m_clickStart;
timeStamp m_clickStop;

float m_clickState;

float m_decay;
float m_increment;
bool m_center;

std::string m_currentRoute;

void init(HCore::HCore *core) {
    m_core = core;
    m_core->log(LOG_INFO, "Init API");

    m_clicking = false;
    m_clickStart = getCurrentTimeMicro();
    m_clickStop = getCurrentTimeMicro();
    m_clickState = 0.0;
    m_increment = 0.0;
    m_decay = 1.0;

    // m_core->eventHandler->addRecurringEvent(getCurrentTimeMicro(),1000,HAPI::checkMouseClick);
    // m_core->eventHandler->addRecurringEvent(getCurrentTimeMicro(),5000,HAPI::decay);
    // m_core->eventHandler->addEvent(getTimeInFuture(1000),checkMouseClick);
    m_center = false;
}

void decay() {
    if (m_clicking) {
        m_clickState += m_increment;
    }
    m_clickState /= m_decay;
}

void setDecay(float value) { m_decay = value; }

void setIncrement(float value) { m_increment = value; }

void setMouseRate(float value) { Workers::mouseWorker->setRate(value); }

void setKeyboardRate(float value) { Workers::keyboardWorker->setRate(value); }

OS::PointerLocation pointerLocation() {
    OS::PointerLocation loc;
    if (m_center) {
        loc.x = Settings::screen.w / 2;
        loc.y = Settings::screen.h / 2;
    } else {
        loc = OS::cursorPos();
    }
    return loc;
}

void ignoreArea(float x0, float y0, float w, float h) { Workers::apiworker->ignoreArea(x0, y0, w, h); }
void clearIgnore() { Workers::apiworker->clearIgnores(); }

void centerPointer(bool value) { m_center = value; }

float getClickState() { return m_clickState; }

int64_t time() { return timeSinceStart(getCurrentTimeMicro()); }

void checkMouseClick() {
    if (timeTo(m_clickStop) <= 0 && m_clicking) {
        m_clicking = false;
        MouseNode node;
        node.code = MOUSE_CLICK_UP;
        Workers::mouseWorker->push(node);
        // OS::mouseUp();
        m_clickStart = getCurrentTimeMicro();
    }
    // m_core->eventHandler->addEvent(getTimeInFuture(1000),checkMouseClick);
}

int timeClicked() {
    if (!m_clicking)
        return 0;
    return timeSince(m_clickStart);
}

void log(const char *logEntry) { m_core->log(LOG_INFO, "API: %s", logEntry); }

void click(uint32_t clickTime) {
    if (!m_clicking) {
        m_clicking = true;
        MouseNode node;
        node.code = MOUSE_CLICK_DOWN;
        Workers::mouseWorker->push(node);
        m_clickStart = getCurrentTimeMicro();
        m_clickStop = m_clickStart + microseconds(clickTime);
    } else {
        m_clickStop = getCurrentTimeMicro() + microseconds(clickTime);
    }
}

void moveMouse(float x, float y, uint32_t usec) {
    MouseNode node;
    node.code = MOUSE_MOVE;
    node.x = x;
    node.y = y;
    node.m_time = getTimeInFuture(usec);
    Workers::mouseWorker->push(node);
}

void clearKeys() { Workers::keyboardWorker->clearListen(); }
void listenKey(uint32_t code) { Workers::keyboardWorker->listen(code); }

bool checkKey(uint32_t code) { return Workers::keyboardWorker->check(code); }

void strictKeyboard(bool val) { Workers::keyboardWorker->setStrict(val); }

void setCaptureOffset(int x, int y, uint32_t utime) { Workers::capture->setOffset(x, y, utime); }

void setCaptureCenter(int x, int y, uint32_t utime) { Workers::capture->setCenter(x, y, utime); }

void setDetectionNetRatio(float value) { Settings::detection.netRatio = value; }

void setNetworkResolution(int width, int height) {
    Settings::net.w = width * 32;
    Settings::net.h = height * 32;
}

void readRoute(std::string name) {
    std::string filename = OS::routesPath + name;
    Util::routeHandler.readRoute(filename);
}

void clearRoutes() { Util::routeHandler.clear(); }

void setRoute(std::string name) { m_currentRoute = name; }

void startCurrentRoute() { Util::routeHandler.reset(m_currentRoute); }

void travelCurrentRoute(uint32_t utime) { Util::routeHandler.travel(m_currentRoute, utime); }

Util::RoutePoint getCurrentLocation() { return Util::routeHandler.getRoute(m_currentRoute).location(); }

void startRoute(std::string name) { Util::routeHandler.reset(name); }

void travelRoute(std::string name, uint32_t utime) { Util::routeHandler.travel(name, utime); }

Util::RoutePoint getLocation(std::string name) { return Util::routeHandler.getRoute(name).location(); }

std::string scriptsPath() { return OS::scriptPath; }

std::string getWeapon() { return Settings::scripts.weaponName; }

std::string getWeaponPath() { return Settings::scripts.weapon; }
std::string getUtilPath() { return Settings::scripts.util; }

void setWeapon(std::string name) { sprintf(Settings::scripts.weaponName, "%s", name.c_str()); }

void sleep(uint32_t usec) { std::this_thread::sleep_for(microseconds(usec)); }

// Train API

void refreshTrainData() {
    Workers::trainer->clearData();
    std::string path = OS::datasetPath + "images/";
    Workers::trainer->readFolder(path);
}

void train(uint32_t iters) {
    Settings::train.iters = iters;
    Workers::trainer->singleLoopAsync();
}

bool training() { return Workers::trainer->isBusy(); }

} // namespace HAPI