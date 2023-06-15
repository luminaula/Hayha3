#include "framebuffer2.hpp"
#include <stdlib.h>
#include <stdint.h>

Framebuffer_t::Framebuffer_t(int width, int height, int x, int y, BufferType type):
    m_width(width),
    m_height(height),
    m_x(x),
    m_y(y),
    m_type(type),
    data(0){
    
    switch (m_type){
        case RGBA:
            data = malloc(sizeof(uint32_t)*m_width*m_height);
            break;
        case YOLO:
            data = malloc(sizeof(float)*m_width*m_height*3);
        default:
            break;
    }

}


Framebuffer_t::~Framebuffer_t(){
    if(data)
        free(data);
}

