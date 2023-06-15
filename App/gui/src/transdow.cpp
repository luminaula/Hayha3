#include "transdow.hpp"
#include "settings.hpp"
#include "buffer.hpp"
#include "workers.hpp"
#include <iostream>
#include "capture.hpp"

TransWindow::TransWindow(QWidget *parent) : QWidget(parent,Qt::SubWindow),
    m_width(Settings::screen.w),
    m_height(Settings::screen.h),
    painter(this){
    setGeometry(0,0,Settings::screen.w,Settings::screen.h);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint | Qt::WindowTransparentForInput | Qt::X11BypassWindowManagerHint | Qt::Widget | Qt::FramelessWindowHint);

    
    setAttribute(Qt::WA_NoSystemBackground, true);

    m_timer = new QTimer(this);
    m_timer->setInterval(1000/Settings::overlay.rate);
    connect(m_timer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));

    m_timer->start();
}

void TransWindow::printStats(Framebuffer &fb){
    QPen penPink(QColor("#FF1493"));
    painter.setPen(penPink);
    QFont serifFont("Helvetica", 18, QFont::DemiBold);
    painter.setFont(serifFont);
    
    char buf[0x200];
    float cTime = (float)getTimeDifference(fb.m_beginProcess,fb.m_beginCapture)/1000.0;
    sprintf(buf,"Capture %d fps, %.2fms",(int)Workers::statServer->getStat("capture"),cTime);
    painter.drawText(10,20,buf);
    float processTime = (float)getTimeDifference(fb.m_endProcess,fb.m_beginProcess)/1000.0;
    sprintf(buf,"Process %.2fms",processTime);
    painter.drawText(10,45,buf);

    float dTime = (float)getTimeDifference(fb.m_endDetect,fb.m_beginDetect)/1000.0;
    sprintf(buf,"Detection %d fps, %.2fms",(int)Workers::statServer->getStat("detect"),dTime);
    painter.drawText(10,70,buf);
    sprintf(buf,"Latency %.2fms",cTime + dTime);
    painter.drawText(10,95,buf);
    float pTime = (float)getTimeDifference(fb.m_beginPresent,fb.m_beginProcess)/1000.0;
    sprintf(buf,"Cap to display %.2fms",pTime);
    painter.drawText(10,120,buf);
    
    sprintf(buf,"Weapon: %s",Settings::scripts.weaponName);
    painter.drawText(10,145,buf);

    HCore::ThreadStateCount tsc = Workers::m_core->tPool->getThreadStates();
    sprintf(buf,"Idle: %d",tsc.idle);
    painter.drawText(10,170,buf);
    sprintf(buf,"Work: %d",tsc.work);
    painter.drawText(10,195,buf);
}

void TransWindow::paintEvent(QPaintEvent *){
    if(!OS::capture->m_bool["isAttached"]())
        return;
    QPen penPink(QColor("#FF1493"));
    char buf[0x200];
    painter.begin(this);
    QFont serifFont2("Helvetica", 14, QFont::DemiBold);
    painter.setFont(serifFont2);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(penPink);
    painter.setBrush(QColor(105, 225, 225, 0));


    Framebuffer &fb = HBuffer::getFramebuffer(HBuffer::PRESENT);

    if(Settings::overlay.showStats)
        printStats(fb);
    if(Settings::overlay.showCrosshair){
        
    }

    uint32_t centerX,centerY;
    centerX = m_width/2;
    centerY = m_height/2;
        
    std::vector<uint32_t> tids = Workers::apiworker->getTargetIds();

    unsigned char r,g,b;
    painter.setPen(penPink);
    for(auto &det : fb.m_detections){

        
        sprintf(buf,"%d : %.2f",det.obj_id,det.prob);
        painter.setPen(penPink);
        painter.drawText(det.x + det.w,det.y+det.h/2,buf);
        
        r = 0;
        g = 255;
        b = 0;
        for(auto &tid : tids){
            if(tid == det.obj_id){
                r = 255;
                g = 0;
                b = 0;
            }
        }
        
        

        painter.setPen(QColor(r,g,b));
        painter.drawRect(det.x-1, det.y-1, det.w + 2,det.h + 2);
        
    }

    painter.end();
    

}