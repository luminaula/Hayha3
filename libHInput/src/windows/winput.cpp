#include <thread>
#include <mutex>
#define NOMINMAX
#include <windows.h>
#include "hinput.hpp"



void moveMouse(int x,int y){
    mouse_event(MOUSEEVENTF_MOVE,x,y,0,0); 
}

void mouseDown(){
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
}

void mouseUp(){
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}


void inputLoop(){

}


void init(){
}

void deinit(){
}

