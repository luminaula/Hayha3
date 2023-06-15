#ifndef MOUSE_HPP
#define MOUSE_HPP

#include <stdint.h>
#include <atomic>
#include "hcore.hpp"
#include "filesystem.hpp"
#include "module.hpp"

namespace OS{

    class HMouse : public Module{
    public:
        HMouse();
    };


    struct PointerLocation{
        int x,y;
    };

    void initMouse();

    PointerLocation cursorPos();
    void moveMouseRel(int x,int y);
    void moveMouseRelCent(int x,int y);
    void moveMouse(int x,int y);

    void mouseDown();
    void mouseUp();
    void checkMouseClick();
    int timeClicked();
    void click(uint32_t clickTime);
}

#endif