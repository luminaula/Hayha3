#include "capturer.hpp"
#include <string.h>
#include "hcore.hpp"
#include "workers.hpp"
#include "capture.hpp"
#include "mouse.hpp"
#include "buffer.hpp"
#include "settings.hpp"
#include "modules.hpp"


Capturer::Capturer(HCore::HCore *core):
        Threadable(Settings::capture.rate, "Capturer",false,core),
        offsetX(0),
        offsetY(0){
    m_limit = false;

    tmp = new unsigned char*[Settings::screen.h];
}

Capturer::~Capturer(){

}

void Capturer::preprocessFramebuffer(Framebuffer &fb){

    Settings::processSettings();
    fb.m_detected = false;
    fb.m_presented = false;
    fb.m_targeted = false;

    fb.ratio = 1.0;
    fb.vertical = false;

    if(Settings::detection.aroundMouse){
        OS::PointerLocation pos = OS::cursorPos();
        Settings::detection.x = (std::max)(0,pos.x - Settings::detection.w/2);
        Settings::detection.y = (std::max)(0,pos.y - Settings::detection.h/2);

    }
    

    if(Settings::detection.x + Settings::detection.w >= Settings::screen.w)
        Settings::detection.x = Settings::screen.w - Settings::detection.w - 5;
    if(Settings::detection.y + Settings::detection.h >= Settings::screen.h)
        Settings::detection.y = Settings::screen.h - Settings::detection.h - 5;

    

    

    fb.detection = Settings::detection;

    if(centerTime > fb.m_beginCapture){
        fb.detection.y = centerY - Settings::detection.h/2;
        fb.detection.x = centerX - Settings::detection.w/2;
    }

    if(offsetTime > fb.m_beginCapture){
        fb.detection.x += offsetX;
        fb.detection.y += offsetY;
    }

    if(Settings::capture.captureDetectionArea){
        Settings::capture.x = fb.detection.x;
        Settings::capture.y = fb.detection.y;
        Settings::capture.w = fb.detection.w;
        Settings::capture.h = fb.detection.h;
    }

    fb.capture = Settings::capture;
    fb.m_tracked.clear();



    if(fb.fWidth != Settings::net.w || fb.fHeight != Settings::net.h ||
            fb.capture.h != fb.height || fb.capture.w != fb.width){
        fb.resize(fb.capture.w,fb.capture.h,Settings::net.w,Settings::net.h,ARGB);
    }

}

void Capturer::processFramebuffer(Framebuffer &fb,unsigned char *buffer){
    fb.m_beginProcess = getCurrentTimeMicro();
    std::vector<FloatArea_t> ignores = Workers::apiworker->getIgnores();
    for(auto &area : ignores){
        //Broken when not capturing whole screen
        int xmin,xmax,ymin,ymax;
        xmin = area.x * Settings::screen.w;
        xmax = xmin + area.w * Settings::screen.w;
        ymin = area.y * Settings::screen.h;
        ymax = ymin + area.h * Settings::screen.h;
        
        xmin -= fb.capture.x;
        ymin -= fb.capture.y;

        fb.drawRectangleFilled(xmin,xmax,ymin,ymax,0x00000000);

        //fb.drawRectangleFilledRel(area.x,area.x+area.w,area)
    }
    //unsigned char *buffer = (unsigned char*)fb.data;

    for(int i=0;i<fb.detection.h;i++){
        tmp[i] =  &buffer[(int)((i+fb.detection.y-fb.capture.y)* fb.width*fb.bytesPerPixel + ((fb.detection.x-fb.capture.x)*fb.bytesPerPixel))];
    }
    
    
    uint16_t threadKey = m_core->generateThreadKey();
    m_core->tPool->resetTaskWaiting(threadKey);
    
    int threads = (std::max)(1,fb.fHeight/64);

    for(int i=0;i<threads;i++){
        int start = (int)(i * ((float)fb.fHeight/threads));
        int end = (int)((float)fb.fHeight/threads*(i+1));                
        m_core->tPool->enqueueBlocking(threadKey,&Framebuffer::scaleRows,fb,start,end,fb.detection.w,fb.detection.h,tmp);
        //fb.scaleRows(start,end,fb.detection.w,fb.detection.h,tmp);
    }

    m_core->tPool->waitForTasks(threadKey,0);
    fb.m_endProcess = getCurrentTimeMicro();

    m_core->tPool->enqueuev(memcpy,fb.data,buffer,fb.width*fb.height*4);
}

void Capturer::setOffset(int x,int y,uint32_t utime){
    offsetX = x;
    offsetY = y;
    offsetTime = getCurrentTimeMicro() + microseconds(utime);
}

void Capturer::setCenter(int x,int y,uint32_t utime){
    centerX = x;
    centerY = y;
    centerTime = getCurrentTimeMicro() + microseconds(utime);
}

void Capturer::work(){
    //m_core->log(LOG_DEBUG,"Begin capture");
    //Get next available framebuffer
    HBuffer::nextFramebuffer(HBuffer::CAPTURE);
    Framebuffer &fb = HBuffer::getFramebuffer(HBuffer::CAPTURE);

    fb.m_beginCapture = getCurrentTimeMicro();
    preprocessFramebuffer(fb);
    //Capture and process framebuffer
    
    unsigned char* buffer = (unsigned char*)OS::captureFrame(fb);
    //
    processFramebuffer(fb,buffer);

    /*
    m_core->tPool->enqueuev([&fb](){
        Modules::tracker->m_void["setDimensions"](fb.capture.x,fb.capture.y,fb.capture.w,fb.capture.h);
        fb.m_tracked = Modules::tracker->m_boxes["trackFrame"](fb.data);
    });
    */

    //notify circular buffer of our latest captured frame
    HBuffer::finishCapture(fb.uid);

    Workers::statServer->pushStat("capture");

    Workers::detect->notify();



}
