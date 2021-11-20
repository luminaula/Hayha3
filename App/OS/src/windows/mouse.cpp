#include <algorithm>
#define NOMINMAX
#include "OScommon.hpp"
#include "mouse.hpp"
#include "settings.hpp"
#include <windows.h>

namespace OS {

PointerLocation cursorPos() {
    POINT p;
    PointerLocation pp;
    GetCursorPos(&p);
    pp.x = p.x;
    pp.y = p.y;

    return pp;
}

void moveMouseRel(int x, int y) {
    PointerLocation p = cursorPos();
    int x0, y0;
    x0 = x - p.x;
    y0 = y - p.y;

    moveMouse(x0, y0);
}

void moveMouseRelCent(int x, int y) {
    PointerLocation p; // = cursorPos();
    p.x = Settings::screen.w / 2;
    p.y = Settings::screen.h / 2;
    int x0, y0;
    x0 = x - p.x;
    y0 = y - p.y;

    moveMouse(x0, y0);
}

} // namespace OS