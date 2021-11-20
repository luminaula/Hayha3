
#include "glwidget.h"

#include <QCoreApplication>
#include <QPaintEngine>

#include "mainwindow.h"
#include "workers.hpp"

#include "INeuralNet.hpp"
#include "buffer.hpp"
#include "hcore.hpp"
#include "mouse.hpp"
#include "settings.hpp"

namespace GUI {

GLWidget::GLWidget()
    : m_width(Settings::net.w), m_height(Settings::net.h), QOpenGLWidget(NULL)
/*m_image(new QImage(m_width,m_height,QImage::Format_RGB32))*/ {

    setMinimumSize(32,32);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Widget);

    m_timer = new QTimer(this);
    m_timer->setInterval(1000 / Settings::display.rate);
    connect(m_timer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));

    m_timer->start();
}

GLWidget::~GLWidget() {
    makeCurrent();
    doneCurrent();
}

void GLWidget::initializeGL() { initializeOpenGLFunctions(); }

void GLWidget::printStats(Framebuffer &fb) {
    QPen penPink(QColor("#FF1493"));
    painter.setPen(penPink);
    QFont serifFont("Helvetica", 18, QFont::DemiBold);
    painter.setFont(serifFont);

    char buf[0x200];
    float cTime = (float)getTimeDifference(fb.m_beginProcess, fb.m_beginCapture) / 1000.0;
    sprintf(buf, "Capture %d fps, %.2fms", (int)Workers::statServer->getStat("capture"), cTime);
    painter.drawText(10, 20, buf);
    float processTime = (float)getTimeDifference(fb.m_endProcess, fb.m_beginProcess) / 1000.0;
    sprintf(buf, "Process %.2fms", processTime);
    painter.drawText(10, 45, buf);

    float dTime = (float)getTimeDifference(fb.m_endDetect, fb.m_beginDetect) / 1000.0;
    sprintf(buf, "Detection %d fps, %.2fms", (int)Workers::statServer->getStat("detect"), dTime);
    painter.drawText(10, 70, buf);
    sprintf(buf, "Latency %.2fms", cTime + dTime);
    painter.drawText(10, 95, buf);
    float pTime = (float)getTimeDifference(fb.m_beginPresent, fb.m_beginProcess) / 1000.0;
    sprintf(buf, "Cap to display %.2fms", pTime);
    painter.drawText(10, 120, buf);

    sprintf(buf, "Weapon: %s", Settings::scripts.weaponName);
    painter.drawText(10, 145, buf);

    HCore::ThreadStateCount tsc = Workers::m_core->tPool->getThreadStates();
    sprintf(buf, "Idle: %d", tsc.idle);
    painter.drawText(10, 170, buf);
    sprintf(buf, "Work: %d", tsc.work);
    painter.drawText(10, 195, buf);
}

bbox_t GLWidget::translateBox(bbox_t det, Framebuffer &fb) {
    /*
    int centerX = m_width / 2 - m_image.width() / 2;
    int centerY = m_height / 2 - m_image.height() / 2;

    float ratioW = (float)m_image.width() / fb.width;
    float ratioH = (float)m_image.height() / fb.height;

    bbox_t bbox;
    bbox.x = (det.x - fb.capture.x) * ratioW + centerX;
    bbox.y = (det.y - fb.capture.y) * ratioH + centerY;
    bbox.w = det.w * ratioW;
    bbox.h = det.h * ratioH;
    */
    return det;
}

void GLWidget::drawDetections(Framebuffer &fb){ /*
    QPen penPink(QColor("#FF1493"));
    char buf[0x200];

    uint32_t centerX, centerY;
    centerX = m_width / 2 - m_image.width() / 2;
    centerY = m_height / 2 - m_image.height() / 2;

    float ratioW = (float)m_image.width() / fb.width;
    float ratioH = (float)m_image.height() / fb.height;

    std::vector<uint32_t> tids = Workers::apiworker->getTargetIds();

    unsigned char r, g, b;
    for (auto &det : fb.m_detections) {
        bbox_t tdet = translateBox(det, fb);
        painter.setPen(penPink);
        sprintf(buf, "%d : %.2f", det.obj_id, det.prob);
        painter.drawText((det.x - fb.capture.x) * ratioW - 10 + centerX, (det.y - fb.capture.y) * ratioH - 10 + centerY, buf);
        r = 0;
        g = 255;
        b = 0;
        for (auto &tid : tids) {
            if (tid == det.obj_id) {
                r = 255;
                g = 0;
                b = 0;
            }
        }

        painter.setPen(QColor(r, g, b));
        painter.drawRect(tdet.x - 1, tdet.y - 1, tdet.w + 2, tdet.h + 2);
    }
    */
}

void GLWidget::drawTarget(Framebuffer &fb) {
    /*

    uint32_t centerX, centerY;
    centerX = m_width / 2 - m_image.width() / 2;
    centerY = m_height / 2 - m_image.height() / 2;

    float ratioW = (float)m_image.width() / fb.width;
    float ratioH = (float)m_image.height() / fb.height;

    int r, g, b;

    r = 255;
    g = 60;
    b = 60;

    painter.setPen(QColor(r, g, b));
    bbox_t currentTarget = Workers::apiworker->getCurrentTarget();
    if (currentTarget.x == 0 && currentTarget.y == 0)
        return;
    int x0 = m_width / 2;
    int y0 = m_height / 2;
    int x1 = ((currentTarget.x + currentTarget.w / 2) - fb.capture.x) * ratioW + centerX;
    int y1 = ((currentTarget.y + currentTarget.h / 2) - fb.capture.y) * ratioH + centerY;

    // x0 *= ratioW;
    x1 *= ratioW;
    // y0 *= ratioH;
    y1 *= ratioH;

    if (x1 != 0 && y1 != 0) {
        for (int i = -1; i < 2; i++) {
            // painter.drawLine(x0-i,y0,x1,y1);
        }
    }
    */
}

void GLWidget::paintGL() {

    // Todo clean this shit up. It's embarassing
    

    Framebuffer &fb = HBuffer::getFramebuffer(HBuffer::PRESENT);
    m_width = fb.fWidth;
    m_height = fb.fHeight;

    if (!fb.m_presented) {
        fb.m_beginPresent = getCurrentTimeMicro();
        images = fb.toQimage();
    }

    setFixedSize(fb.fWidth*3, fb.fHeight*3);
    QRect paintArea = QRect(0,0,fb.fWidth*3, fb.fHeight*3);
    
    painter.begin(this);
    painter.setWindow(paintArea);
    //painter.beginNativePainting();
    

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int indx = 0, indy = 0;
    int imagesPerColumn = 3;
    for(auto &image :images){
        QRect targetArea(indx*fb.fWidth,indy*fb.fHeight,fb.fWidth,fb.fHeight);
        painter.drawImage(targetArea,image);
        indx++;
        if(indx >= imagesPerColumn){
            indx = 0;
            indy++;
        }
        
        
    }
    /*
    for(int i=0;i<fb.channels;i++){
        
        QRect targetArea(indx,indy,images[i].width(),images[i].height());
        
        //painter.drawImage(targetArea,images[i]);
        
        painter.drawImage(targetArea,images[i]);

        
        indx += images[i].width();
        if(indx >= fb.fWidth*3){
            indx = 0;
            indy += images[i].height();
        }
        
        

        fb.m_presented = true;

        

    }
    */

    //painter.endNativePainting();

        /*
        drawDetections(fb);
        // printStats(fb);
        // drawTarget(fb);

        QFont serifFont2("Helvetica", 14, QFont::DemiBold);
        painter.setFont(serifFont2);

        painter.setPen(QColor(75, 255, 40));
        int x0 = (fb.detection.x - fb.capture.x) * ratioW + centerX;
        int y0 = (fb.detection.y - fb.capture.y) * ratioH + centerY;
        int w0 = fb.detection.w * ratioW;
        int h0 = fb.detection.h * ratioH;
        painter.drawRect(x0, y0, w0, h0);
        painter.drawRect(x0 - 1, y0 - 1, w0 + 2, h0 + 2);

    */
    painter.end();
    


    // delete m_image;
}

DetectionWindow::DetectionWindow(MainWindow *mw) : mainWindow(mw) {
    m_glWidget = new GLWidget;
    m_glWidget->setParent(this);

    setWindowTitle(tr("DetectionView"));

    m_timer = new QTimer(this);
    m_timer->setInterval(1000 / Settings::display.rate);
    connect(m_timer, &QTimer::timeout, m_glWidget, QOverload<>::of(&QWidget::update));

    m_timer->start();
}

void DetectionWindow::updateIntervalChanged(int value) {
    if (value > 144)
        value = 144;
    m_timer->setInterval(1000 / value);
    if (m_timer->isActive())
        m_timer->start();
}

} // namespace GUI