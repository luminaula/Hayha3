#include "framebuffer.hpp"
#include "image.hpp"
#include <string.h>
#include <QImage>


static inline int fastfloor(float fp) {
    int i = static_cast<int>(fp);
    return (fp < i) ? (i - 1) : (i);
}


Framebuffer::Framebuffer(int width_,int height_,int fWidth_,int fHeight_,PixelFormat fmt):
        format(fmt),
        m_detected(false),
        width(width_),
        height(height_),
        fWidth(fWidth_),
        fHeight(fHeight_),
        rWidth(fWidth_),
        rHeight(fHeight){

    data = new int[width*height];
    fdata = new float[fWidth*fHeight*3];
    rdata = new unsigned char[fWidth*fHeight*3];

    clearMemory(0x00);
    clearFloatMemory();

    if(format == RGB){
        setPixel = &Framebuffer::setPixelRGB;
        getPixel = &Framebuffer::getPixelRGB;
        bytesPerPixel = 3;
    }
    else if(format == BGR){
        setPixel = &Framebuffer::setPixelBGR;
        getPixel = &Framebuffer::getPixelBGR;
        bytesPerPixel = 3;
    }
    else if(format == ARGB){
        setPixel = &Framebuffer::setPixelARGB;
        getPixel = &Framebuffer::getPixelARGB;
        bytesPerPixel = 4;
    }

    

    if(format == RGB){
        fColors[0] = fdata;
        fColors[1] = fColors[0] + fWidth * fHeight;
        fColors[2] = fColors[1] + fWidth * fHeight;
    }
    else if(format == BGR){
        fColors[2] = fdata;
        fColors[1] = fColors[2] + fWidth * fHeight;
        fColors[0] = fColors[1] + fWidth * fHeight;
    }
    else{
        fColors[2] = fdata;
        fColors[1] = fColors[2] + fWidth * fHeight;
        fColors[0] = fColors[1] + fWidth * fHeight;
    }

    m_mutex = new std::mutex();

    

}

Framebuffer::~Framebuffer(){
    //TODO Fix this
    /*
    if(data != NULL)
        delete[]data;
    if(fdata != NULL)
        delete[]fdata;
    */
}


void Framebuffer::clearMemory(char shade){
    memset(data,shade,width*height*4);
}

void Framebuffer::clearFloatMemory(){
    memset(fdata,0,fWidth*fHeight*3);
}

void Framebuffer::resize(int width_,int height_,int fWidth_,int fHeight_,PixelFormat fmt){
    std::lock_guard<std::mutex> lock(*m_mutex);
    width = width_;
    height = height_;
    fWidth = fWidth_;
    fHeight = fHeight_;
    rWidth = fWidth_;
    rHeight = fHeight_;
    
    if(data) delete[]data;
    if(fdata) delete[]fdata;
    if(rdata) delete[]rdata;
    data = new int[width*height];
    fdata = new float[fWidth*fHeight*3];
    rdata = new unsigned char[fWidth*fHeight*3];

    clearMemory(0x00);
    clearFloatMemory();

}

void Framebuffer::setPixelRGB(int x0,int y0,uint32_t color){
    unsigned char *tmp = (unsigned char*)data;
    tmp += y0*width*3 + x0*3;
    tmp[0] = color >> 16 & 0xff;
    tmp[1] = color >> 8 & 0xff;
    tmp[2] = color & 0xff;
}
void Framebuffer::setPixelBGR(int x0,int y0,uint32_t color){
    unsigned char *tmp = (unsigned char*)data;
    tmp += y0*width*3 + x0*3;
    tmp[2] = color >> 16 & 0xff;
    tmp[1] = color >> 8 & 0xff;
    tmp[0] = color & 0xff;
}
void Framebuffer::setPixelARGB(int x0,int y0,uint32_t color){
    if(x0 >= width || x0 < 0 || y0 >= height || y0 < 0)
        return;
    
    int *tmp = (int*)data;
    tmp[y0*width + x0] = color;
}

int Framebuffer::getPixelRGB(int x0,int y0){
    uint32_t color = 0;
    unsigned char *tmp = (unsigned char*)data;
    tmp += y0*width*3 + x0*3;
    color |= tmp[0] << 16;
    color |= tmp[1] << 8;
    color |= tmp[2];

    return color;

}

int Framebuffer::getPixelBGR(int x0,int y0){
    uint32_t color = 0;
    unsigned char *tmp = (unsigned char*)data;
    tmp += y0*width*3 + x0*3;
    color |= tmp[2] << 16;
    color |= tmp[1] << 8;
    color |= tmp[0];

    return color;

}

int Framebuffer::getPixelARGB(int x0,int y0){
    int *tmp = (int*)data;
    return tmp[y0 *width + x0];
}


void Framebuffer::drawRectangleFilled(int xmin, int xmax, int ymin, int ymax, uint32_t color){
    xmin = (std::min)(xmin,(int)width);
    xmax = (std::min)(xmax,(int)width);
    ymin = (std::min)(ymin,(int)height);
    ymax = (std::min)(ymax,(int)height);

    xmin = (std::max)(xmin,0);
    xmax = (std::max)(xmax,0);
    ymin = (std::max)(ymin,0);
    ymax = (std::max)(ymax,0);


    for(int i=ymin;i<ymax;i++){
        for(int j=xmin;j<xmax;j++){
            setPixelARGB(j,i,color);
        }
    }

}


void Framebuffer::drawRectangleFilledRel(float xmin, float xmax, float ymin, float ymax, uint32_t color){
    drawRectangleFilled(xmin*width,xmax*width,ymin*width,ymax*width,color);
}


void Framebuffer::scaleRow(int row, int width,int height, unsigned char **tmp){
    int aheight = fHeight;
    int awidth = fWidth;
    
 
    
    if(row >= aheight)
        return;
    

    float *colore[3];
    unsigned char *rColore;
    if(format == RGB){
        colore[0] = fdata + row * fWidth;
        colore[1] = colore[0] + fWidth * fHeight;
        colore[2] = colore[1] + fWidth * fHeight;

    }
    else if(format == BGR){
        colore[2] = fdata + row * fWidth;
        colore[1] = colore[2] + fWidth * fHeight;
        colore[0] = colore[1] + fWidth * fHeight;

    }
    else if(format == ARGB){
        //Lil endian
        colore[2] = fdata + row * fWidth;
        colore[1] = colore[2] + fWidth * fHeight;
        colore[0] = colore[1] + fWidth * fHeight;

        rColore = rdata + row * rWidth * 3;

    }
    


    for(int x0=0;x0<awidth;x0++){
        float gx = x0 / (float)(awidth) * (width);
        float gy = row / (float)(aheight) * (height);

        int gxi = fastfloor(gx);
        int gyi = fastfloor(gy);

        
        for(int c=0;c<3;c++){
            
            unsigned char c00 = tmp[gyi][gxi*bytesPerPixel+c];
            /*
            unsigned char c10 = tmp[gyi][(gxi+1)*bytesPerPixel+c];
            unsigned char c01 = tmp[gyi][gxi*bytesPerPixel+c];
            unsigned char c11 = tmp[gyi][(gxi+1)*bytesPerPixel+c];
            
            float blerpColor = blerp(c00,c10,c01,c11,gx-gxi,gy-gyi);
            *colore[c]++ = blerpColor/255.0; 
            *rColore[c]++ = (unsigned char)blerpColor;
            */
            *rColore++ = c00;
            *colore[c]++ = (float)c00/255.0;


            //*colore[c]++ = (float)tmp[gyi][gxi*bytesPerPixel+c]/ 255.0;
            //*rColore[c]++ = tmp[gyi][gxi*bytesPerPixel+c];
        }
        
    }


}

void Framebuffer::scaleRows(int start, int end,int width,int height, unsigned char **tmp){
    for(int i = start;i<end;i++){
        scaleRow(i,width,height,tmp);
    }
}

void Framebuffer::toQimage(QImage *qimg){
    std::lock_guard<std::mutex> lock(*m_mutex);

    int height_ = detection.h;
    int width_ = detection.w;

    int offsetX = detection.x - capture.x;
    int offsetY = detection.y - capture.y;


    
    for(int i=0;i<height_;i++){
        for(int j=0;j<width_;j++){
            uint32_t color = 0;
            float gx = j / (float)(width_) * (rWidth);
            float gy = i / (float)(height_) * (rHeight);

            int gxi = fastfloor(gx);
            int gyi = fastfloor(gy);

            for(int c=0;c<3;c++){
                
                float c00,c01,c10,c11;

                c00 = fdata[gyi*fWidth + gxi + (c*fWidth*fHeight)];
                /*
                c01 = fdata[gyi*fWidth + gxi+1 + (c*fWidth*fHeight)];
                c10 = fdata[(gyi+1)*fWidth + gxi + (c*fWidth*fHeight)];
                c11 = fdata[(gyi+1)*fWidth + gxi+1 + (c*fWidth*fHeight)];
                */

                //color |= (unsigned char)(blerp(c00,c10,c01,c11,gx-gxi,gy-gyi)*255.0) << (16-(c * 8));
                color |= (unsigned char)(c00*255.0) << (16-(c * 8));
                
                //color |= (unsigned char)(rdata[gyi*rWidth + gxi + (c*rWidth*rHeight)]) << (16-(c*8));

            }
            setPixelARGB(j+offsetX,i+offsetY,color);
            
        }
    }

    int start = 0;
    int end = qimg->height();
    unsigned int *data = (unsigned int*)qimg->bits();
    for(int i=start;i<end;i++){
        for(int j=0;j<qimg->width();j++){
            float gx = j / (float)(qimg->width()) * (this->width);
            float gy = i / (float)(qimg->height()) * (this->height);
            data[i * qimg->width() + j] = getPixelARGB(fastfloor(gx),fastfloor(gy));
        }
    }

    



}
