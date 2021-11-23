#include "framebuffer.hpp"
#include "image.hpp"
#include <QImage>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <fstream>
#include <iostream>

#include "colorspaces.hpp"

static inline int fastfloor(float fp) {
    int i = static_cast<int>(fp);
    return (fp < i) ? (i - 1) : (i);
}

Framebuffer::Framebuffer(int width_, int height_, int fWidth_, int fHeight_, PixelFormat fmt)
    : format(fmt), m_detected(false), width(width_), height(height_), fWidth(fWidth_), fHeight(fHeight_), rWidth(fWidth_),
      rHeight(fHeight) {

    data = new int[width * height];
    fdata = new float[fWidth * fHeight * 9];
    rdata = new unsigned char[fWidth * fHeight * 3];
    channels = 9;

    rgbdata = fdata;
    xyzdata = rgbdata + fWidth * fHeight * 3;
    labdata = xyzdata + fWidth * fHeight * 3;

    clearMemory(0x00);
    clearFloatMemory();

    if (format == RGB) {
        setPixel = &Framebuffer::setPixelRGB;
        getPixel = &Framebuffer::getPixelRGB;
        bytesPerPixel = 3;
    } else if (format == BGR) {
        setPixel = &Framebuffer::setPixelBGR;
        getPixel = &Framebuffer::getPixelBGR;
        bytesPerPixel = 3;
    } else if (format == ARGB) {
        setPixel = &Framebuffer::setPixelARGB;
        getPixel = &Framebuffer::getPixelARGB;
        bytesPerPixel = 4;
    }

    if (format == RGB) {
        fColors[0] = fdata;
        fColors[1] = fColors[0] + fWidth * fHeight;
        fColors[2] = fColors[1] + fWidth * fHeight;
    } else if (format == BGR) {
        fColors[2] = fdata;
        fColors[1] = fColors[2] + fWidth * fHeight;
        fColors[0] = fColors[1] + fWidth * fHeight;
    } else {
        fColors[2] = fdata;
        fColors[1] = fColors[2] + fWidth * fHeight;
        fColors[0] = fColors[1] + fWidth * fHeight;
    }

    m_mutex = new std::mutex();
}

Framebuffer::~Framebuffer() {
    // TODO Fix this
    /*
    if(data != NULL)
        delete[]data;
    if(fdata != NULL)
        delete[]fdata;
    */
}

void Framebuffer::clearMemory(char shade) { memset(data, shade, width * height * 4); }

void Framebuffer::clearFloatMemory() { memset(fdata, 0, fWidth * fHeight * 3); }

void Framebuffer::resize(int width_, int height_, int fWidth_, int fHeight_, PixelFormat fmt) {
    std::lock_guard<std::mutex> lock(*m_mutex);
    width = width_;
    height = height_;
    fWidth = fWidth_;
    fHeight = fHeight_;
    rWidth = fWidth_;
    rHeight = fHeight_;

    if (data)
        delete[] data;
    if (fdata)
        delete[] fdata;
    if (rdata)
        delete[] rdata;
    data = new int[width * height];
    fdata = new float[fWidth * fHeight * 9];
    rdata = new unsigned char[fWidth * fHeight * 3];

    clearMemory(0x00);
    clearFloatMemory();
}

void Framebuffer::setPixelRGB(int x0, int y0, uint32_t color) {
    unsigned char *tmp = (unsigned char *)data;
    tmp += y0 * width * 3 + x0 * 3;
    tmp[0] = color >> 16 & 0xff;
    tmp[1] = color >> 8 & 0xff;
    tmp[2] = color & 0xff;
}
void Framebuffer::setPixelBGR(int x0, int y0, uint32_t color) {
    unsigned char *tmp = (unsigned char *)data;
    tmp += y0 * width * 3 + x0 * 3;
    tmp[2] = color >> 16 & 0xff;
    tmp[1] = color >> 8 & 0xff;
    tmp[0] = color & 0xff;
}
void Framebuffer::setPixelARGB(int x0, int y0, uint32_t color) {
    if (x0 >= width || x0 < 0 || y0 >= height || y0 < 0)
        return;

    int *tmp = (int *)data;
    tmp[y0 * width + x0] = color;
}

int Framebuffer::getPixelRGB(int x0, int y0) {
    uint32_t color = 0;
    unsigned char *tmp = (unsigned char *)data;
    tmp += y0 * width * 3 + x0 * 3;
    color |= tmp[0] << 16;
    color |= tmp[1] << 8;
    color |= tmp[2];

    return color;
}

int Framebuffer::getPixelBGR(int x0, int y0) {
    uint32_t color = 0;
    unsigned char *tmp = (unsigned char *)data;
    tmp += y0 * width * 3 + x0 * 3;
    color |= tmp[2] << 16;
    color |= tmp[1] << 8;
    color |= tmp[0];

    return color;
}

int Framebuffer::getPixelARGB(int x0, int y0) {
    int *tmp = (int *)data;
    return tmp[y0 * width + x0];
}

void Framebuffer::drawRectangleFilled(int xmin, int xmax, int ymin, int ymax, uint32_t color) {
    xmin = (std::min)(xmin, (int)width);
    xmax = (std::min)(xmax, (int)width);
    ymin = (std::min)(ymin, (int)height);
    ymax = (std::min)(ymax, (int)height);

    xmin = (std::max)(xmin, 0);
    xmax = (std::max)(xmax, 0);
    ymin = (std::max)(ymin, 0);
    ymax = (std::max)(ymax, 0);

    for (int i = ymin; i < ymax; i++) {
        for (int j = xmin; j < xmax; j++) {
            setPixelARGB(j, i, color);
        }
    }
}

void Framebuffer::drawRectangleFilledRel(float xmin, float xmax, float ymin, float ymax, uint32_t color) {
    drawRectangleFilled(xmin * width, xmax * width, ymin * width, ymax * width, color);
}

void Framebuffer::scaleRow(int row, int width, int height, unsigned char **tmp) {
    int aheight = fHeight;
    int awidth = fWidth;

    if (row >= aheight)
        return;


    float *rgbp[3];
    float *xyzp[3];
    float *labp[3];
    float *hsvp[3];
    
    unsigned char *rColore;
    rgbp[0] = fdata + row * fWidth;
    rgbp[1] = rgbp[0] + fWidth * fHeight;
    rgbp[2] = rgbp[1] + fWidth * fHeight;

    xyzp[0] = rgbp[2] + fWidth * fHeight;
    xyzp[1] = xyzp[0] + fWidth * fHeight;
    xyzp[2] = xyzp[1] + fWidth * fHeight;

    labp[0] = xyzp[2] + fWidth * fHeight;
    labp[1] = labp[0] + fWidth * fHeight;
    labp[2] = labp[1] + fWidth * fHeight;

    HEBI::Colors::Color *rgb = &HEBI::Colors::rgb[row * fWidth];

    rColore = rdata + row * rWidth * 3;
    

    for (int x0 = 0; x0 < awidth; x0++) {
        float gx = x0 / (float)(awidth) * (width);
        float gy = row / (float)(aheight) * (height);

        int gxi = fastfloor(gx);
        int gyi = fastfloor(gy);

        unsigned int color = 0;

        float r,g,b,x,y,z,l,a,bb;
        for (int c = 0; c < 3; c++) {
            unsigned char c00 = tmp[gyi][gxi * bytesPerPixel + c];
            /*
            unsigned char c10 = tmp[gyi][(gxi+1)*bytesPerPixel+c];
            unsigned char c01 = tmp[gyi][gxi*bytesPerPixel+c];
            unsigned char c11 = tmp[gyi][(gxi+1)*bytesPerPixel+c];

            /*
            *colore[c]++ = blerpColor/255.0;
            *rColore[c]++ = (unsigned char)blerpColor;
            */

            //*rgbp[2-c] = (float)c00 / 255.0;
            *rColore++ = c00;
            
            //unsigned char blerpColor = blerp(c00,c10,c01,c11,gx-gxi,gy-gyi);

            color |= c00 << (c * 8);

        }

        HEBI::Colors::Color rgb = HEBI::Colors::rgb[color];
        //HEBI::Colors::Color xyz = HEBI::Colors::xyz[color];
        //HEBI::Colors::Color lab = HEBI::Colors::lab[color];

        *rgbp[0]++ = rgb.values[0];
        *rgbp[1]++ = rgb.values[1];
        *rgbp[2]++ = rgb.values[2];

        //*xyzp[0]++ = xyz.values[0];
        //*xyzp[1]++ = xyz.values[1];
        //*xyzp[2]++ = xyz.values[2];
        
        //*labp[0]++ = lab.values[0];
        //*labp[1]++ = lab.values[1];
        //*labp[2]++ = lab.values[2];


    }
}

void Framebuffer::scaleRows(int start, int end, int width, int height, unsigned char **tmp) {
    for (int i = start; i < end; i++) {
        scaleRow(i, width, height, tmp);
    }
}

std::vector<QImage> Framebuffer::toQimage(){
    std::lock_guard<std::mutex> lock(*m_mutex);

    

    std::vector<QImage> images;

    float *channel = fdata;
    for(int i=0;i<channels;i++){
        QImage img(fWidth,fHeight,QImage::Format_Grayscale8);
        unsigned char *bits = img.bits();
        for(int j=0;j<fWidth*fHeight;j++){
            *bits++ = *channel++ * 255.0;
        }
        images.push_back(img);
    }

    /*
    int start = 0;
    int end = qimg->height();
    unsigned int *data = (unsigned int *)qimg->bits();
    for (int i = start; i < end; i++) {
        for (int j = 0; j < qimg->width(); j++) {
            float gx = j / (float)(qimg->width()) * (this->width);
            float gy = i / (float)(qimg->height()) * (this->height);
            data[i * qimg->width() + j] = getPixelARGB(fastfloor(gx), fastfloor(gy));
        }
    }
    */
   return images;
}

void Framebuffer::save(std::string &filename) {
    unsigned char *tmpbuffer = new unsigned char[rWidth * rHeight * 3];
    for (int i = 0; i < rWidth * rHeight * 3; i += 3) {
        tmpbuffer[i] = rdata[i + 2];
        tmpbuffer[i + 1] = rdata[i + 1];
        tmpbuffer[i + 2] = rdata[i];
    }
    stbi_write_jpg(filename.c_str(), rWidth, rHeight, 3, tmpbuffer, 100);
    delete[] tmpbuffer;
}

void Framebuffer::saveAnnotation(std::string &filename, std::string &imageName) {
    std::ofstream annotationFile;
    annotationFile.open(filename);
    annotationFile << "<annotation>" << std::endl;
    annotationFile << "<imageName>" << imageName << "</imageName>" << std::endl;
    annotationFile << "<width>" << rWidth << "</width>" << std::endl;
    annotationFile << "<height>" << rHeight << "</height>" << std::endl;

    for (auto &box : m_detections) {
        annotationFile << "<object>" << std::endl;
        annotationFile << "\t<objName>" << box.obj_id << "</objName>" << std::endl;
        annotationFile << "\t<xmin>" << box.x - capture.x << "</xmin>" << std::endl;
        annotationFile << "\t<xmax>" << box.x + box.w - capture.x << "</xmax>" << std::endl;
        annotationFile << "\t<ymin>" << box.y - capture.y << "</ymin>" << std::endl;
        annotationFile << "\t<ymax>" << box.y + box.h - capture.y << "</ymax>" << std::endl;
        annotationFile << "</object>" << std::endl;
    }
    annotationFile << "</annotation>" << std::endl;
    annotationFile.close();
}

void Framebuffer::saveAnnotationYOLO(std::string &filename) {
    std::ofstream annotationFile;
    annotationFile.open(filename);

    for (auto &box : m_detections) {
        box.x -= capture.x;
        box.y -= capture.y;
        float x, y, w, h, dw, dh;

        dw = 1.0 / rWidth;
        dh = 1.0 / rHeight;

        x = box.x + box.w / 2 - 1;
        y = box.y + box.h / 2 - 1;

        w = box.w;
        h = box.h;

        x *= dw;
        w *= dw;

        y *= dh;
        h *= dh;

        annotationFile << box.obj_id << " " << x << " " << y << " " << w << " " << h << std::endl;
    }

    annotationFile.close();
}