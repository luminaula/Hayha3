#include <X11/Xlib.h>

namespace OS {
extern Display *m_display;
extern Window m_root;
extern Window m_window;
extern XWindowAttributes m_window_attributes;
extern Screen *m_screen;
} // namespace OS