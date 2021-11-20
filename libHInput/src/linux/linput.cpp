#include <thread>
#include <mutex>
#include "hinput.hpp"

#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include <linux/input.h>
#include <linux/uinput.h>
#include <fcntl.h>
#include <errno.h>



std::thread *inputThread;
bool running = false;

struct uinput_user_dev udev;
struct input_event inputEv;
int fd;
bool mouseInited;



void init(){

    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        mouseInited = false;
        printf("Mouse init failed\n");
        printf("Use sudo chmod 0666 /dev/uinput\n");
        return;
    }

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);

    ioctl(fd, UI_SET_EVBIT, EV_REL);
    ioctl(fd, UI_SET_RELBIT, REL_X);
    ioctl(fd, UI_SET_RELBIT, REL_Y);

    memset(&udev, 0, sizeof(udev));
    
    //Todo maybe allow user to change these or something idk 
    snprintf(udev.name,UINPUT_MAX_NAME_SIZE,"Hebi mx420");
    udev.id.bustype = BUS_USB;
    udev.id.vendor = 0x420;
    udev.id.product = 0x69;
    udev.id.version = 1;

    udev.absmax[REL_X] = 512;
    udev.absmin[REL_X] = -512;
    udev.absfuzz[REL_X] = 0;
    udev.absflat[REL_X] = 15;

    udev.absmax[REL_Y] = 512;
    udev.absmin[REL_Y] = -512;
    udev.absfuzz[REL_Y] = 0;
    udev.absflat[REL_Y] = 15;

    //Todo Error handling
    if(write(fd,&udev,sizeof(udev))<0){
    }

    if(ioctl(fd, UI_DEV_CREATE) < 0){
    }
    mouseInited = true;

    running = true;
    inputThread = new std::thread(inputLoop);

}

void moveMouse(int x,int y){
    
    if(!mouseInited){
        init();
        if(!mouseInited)
            return;
    }
    int stopWarningMe;

    memset(&inputEv, 0, sizeof(struct input_event));

    if(x != 0){
        inputEv.type = EV_REL;
        inputEv.code = REL_X;
        inputEv.value = x;
        stopWarningMe = write(fd,&inputEv,sizeof(struct input_event));
        x -= inputEv.value;
    }
    if(y != 0){
        inputEv.type = EV_REL;
        inputEv.code = REL_Y;
        inputEv.value = y;
        stopWarningMe = write(fd,&inputEv,sizeof(struct input_event));
        y -= inputEv.value;
    }
    

}

void mouseDown(){
    int stopWarningMe;
    memset(&inputEv, 0, sizeof(struct input_event));
    inputEv.type = EV_KEY;
    inputEv.code = BTN_LEFT;
    inputEv.value = 1;
    stopWarningMe = write(fd,&inputEv,sizeof(struct input_event));
    inputEv.type = EV_SYN;
    inputEv.code = SYN_REPORT;
    inputEv.value = 0;
    stopWarningMe = write(fd,&inputEv,sizeof(struct input_event));
}

void mouseUp(){

    int stopWarningMe;
    inputEv.type = EV_KEY;
    inputEv.code = BTN_LEFT;
    inputEv.value = 0;
    stopWarningMe = write(fd,&inputEv,sizeof(struct input_event));
    inputEv.type = EV_SYN;
    inputEv.code = SYN_REPORT;
    inputEv.value = 0;
    stopWarningMe = write(fd,&inputEv,sizeof(struct input_event));
}


void inputLoop(){
    //Do something here
    while(running){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}


void deinit(){
    running = false;
    inputThread->join();
    delete inputThread;
}

