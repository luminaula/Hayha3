#include "X11common.hpp"
#include "OScommon.hpp"
#include "capture.hpp"
#include "hcore.hpp"
#include "mouse.hpp"

namespace OS {
Display *m_display;
Window m_root;
Window m_window;
XWindowAttributes m_window_attributes;
Screen *m_screen;

int XerrorHandler(Display *d, XErrorEvent *e) {
    capture->m_void["Error"](e);

    return 0;
}

void init(HCore::HCore *core) {
    m_core = core;
    m_core->log(LOG_INFO, "Init X");
    XInitThreads();
    XSetErrorHandler(OS::XerrorHandler);
    m_display = XOpenDisplay(nullptr);
    m_root = DefaultRootWindow(m_display);

    initCapture();
    initMouse();
}

} // namespace OS