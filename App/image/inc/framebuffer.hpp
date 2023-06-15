#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include <stdint.h>
#include "time.hpp"
#include <functional>
#include "INeuralNet.hpp"
#include "settings.hpp"
#include <memory>
#include <mutex>


class QImage;

enum PixelFormat{
    ARGB,
    RGB,
    BGR
};


struct Framebuffer{

    int rWidth;
    int rHeight;
    
    int fWidth;
	int fHeight;
	
    float *fColors[3];
    int width;
	int height;
	
    int bytesPerPixel;
    PixelFormat format;

    int *data;
    float *fdata;
    unsigned char *rdata;

    float ratio;
    bool vertical;
    int uid;

    std::mutex *m_mutex;

    Settings::DetectionSettings_t detection;
    Settings::CaptureSettings_t capture;

    std::vector<bbox_t> m_detections;
    std::vector<bbox_t> m_detectionsPrev;
    std::vector<bbox_t> m_tracked;


    bool m_detected;
    bool m_targeted;
    bool m_presented;

    timeStamp m_beginCapture;
    timeStamp m_beginProcess;
    timeStamp m_endProcess;
    timeStamp m_beginDetect;
    timeStamp m_endDetect;
    timeStamp m_beginPresent;

    Framebuffer();
    Framebuffer(int width_,int height_,int fWidth_,int fHeight_,PixelFormat fmt);
    ~Framebuffer();


    void clearFloatMemory();

    void clearMemory(char shade);

    void (Framebuffer::*setPixel)(int x,int y,uint32_t color);
    int (Framebuffer::*getPixel)(int x,int y);

    void setPixelRGB(int x0,int y0,uint32_t color);
    void setPixelBGR(int x0,int y0,uint32_t color);
    void setPixelARGB(int x0,int y0,uint32_t color);

    int getPixelRGB(int x0,int y0);
    int getPixelBGR(int x0,int y0);
    int getPixelARGB(int x0,int y0);

    void drawRectangleFilled(int xmin, int xmax, int ymin, int ymax, uint32_t color);
    void drawRectangleFilledRel(float xmin, float xmax, float ymin, float ymax, uint32_t color);


    void scaleRow(int row, int width,int height, unsigned char **tmp);
    void scaleRows(int start, int end, int width,int height, unsigned char **tmp);

    void resize(int width_,int height_,int fWidth_,int fHeight_,PixelFormat fmt);

    void copyFramebuffer(Framebuffer &fb);
    void toQimage(QImage *qimg);



};

#endif