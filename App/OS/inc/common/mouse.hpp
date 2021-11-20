#ifndef MOUSE_HPP
#define MOUSE_HPP

#include "filesystem.hpp"
#include "hcore.hpp"
#include "module.hpp"
#include <atomic>
#include <stdint.h>

namespace OS {

class HMouse : public Module {
  public:
    HMouse();
};

struct PointerLocation {
    int x, y;
};

void initMouse();

PointerLocation cursorPos();
void moveMouseRel(int x, int y);
void moveMouseRelCent(int x, int y);
void moveMouse(int x, int y);

void mouseDown();
void mouseUp();
void checkMouseClick();
int timeClicked();
void click(uint32_t clickTime);
} // namespace OS

#endif