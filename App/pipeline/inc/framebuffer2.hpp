#pragma once

#include <vector>


enum BufferType{
    RGBA,
    YOLO
};


struct Framebuffer_t{
    BufferType m_type;
    int m_width, m_height, m_x, m_y;
    void *data;

    Framebuffer_t(int width, int height, int x, int y, BufferType type);
    ~Framebuffer_t();

};

