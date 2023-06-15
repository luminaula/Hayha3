#include "mouse.hpp"
#include "X11common.hpp"
#include "hcore.hpp"
#include "OScommon.hpp"

#include <X11/XKBlib.h>

#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include <linux/input.h>
#include <linux/uinput.h>
#include <fcntl.h>
#include <errno.h>
#include <algorithm>
#include "settings.hpp"
#include "OScommon.hpp"



namespace OS{
    
    
    struct uinput_user_dev udev;
    struct input_event inputEv;
    int fd;
    bool mouseInited;

    PointerLocation cursorPos(){
        PointerLocation p;
        int win_x, win_y;
        int result;
        unsigned int mask;
        Window retWin;
        result = XQueryPointer(m_display, m_root, &retWin,
                &retWin, &p.x, &p.y, &win_x, &win_y,
                &mask);
        return p;
    }


    void moveMouseRel(int x,int y){
        PointerLocation p = cursorPos();
        int x0,y0;
        x0 = x-p.x;
        y0 = y-p.y;

        /*mainCore->tPool->enqueuev(*/
        moveMouse(x0,y0);

    }

    void moveMouseRelCent(int x,int y){
        PointerLocation p;// = cursorPos();        
        p.x = Settings::screen.w/2;
        p.y = Settings::screen.h/2;
        int x0,y0;
        x0 = x-p.x;
        y0 = y-p.y;

        /*mainCore->tPool->enqueuev(*/
        moveMouse(x0,y0);

    }



}