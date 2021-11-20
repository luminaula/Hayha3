#include "mouse.hpp"
#include "OScommon.hpp"
#include "filesystem.hpp"

namespace OS {

HMouse::HMouse() {
    std::string filename = OS::libPath + OS::libPrefix + "HInput" + OS::libSuffix;
    loadLibrary(filename.c_str());
    m_void["init"] = loadFunc<void>("init");
    m_void["moveMouse"] = loadFunc<void>("moveMouse");
    m_void["mouseDown"] = loadFunc<void>("mouseDown");
    m_void["mouseUp"] = loadFunc<void>("mouseUp");
}

HMouse *mouse;
std::mutex mouseMutex;

void initMouse() {
    mouse = new HMouse();
    mouse->m_void["init"]();
}

void moveMouse(int x, int y) {
    std::lock_guard<std::mutex> lock(mouseMutex);
    mouse->m_void["moveMouse"](x, y);
}
void mouseDown() {
    std::lock_guard<std::mutex> lock(mouseMutex);
    mouse->m_void["mouseDown"]();
}
void mouseUp() {
    std::lock_guard<std::mutex> lock(mouseMutex);
    mouse->m_void["mouseUp"]();
}

} // namespace OS