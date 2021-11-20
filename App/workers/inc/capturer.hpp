#ifndef CAPTURE_HPP
#define CAPTURE_HPP

#include "hcore.hpp"

struct Framebuffer;

class Capturer : public Threadable {
  private:
    int offsetX, offsetY;
    int centerX, centerY;
    timeStamp centerTime, offsetTime;
    unsigned char **tmp;

    void preprocessFramebuffer(Framebuffer &fb);
    void processFramebuffer(Framebuffer &fb, unsigned char *buffer);

  public:
    Capturer(HCore::HCore *core);
    ~Capturer();

    void setOffset(int x, int y, uint32_t utime);
    void setCenter(int x, int y, uint32_t utime);

    void work();
};

#endif