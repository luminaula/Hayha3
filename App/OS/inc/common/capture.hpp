#pragma once

#include "framebuffer.hpp"
#include "module.hpp"

namespace OS {

class HCapture : public Module {
  public:
    HCapture();
};

extern HCapture *capture;

void initCapture();
void *captureFrame(Framebuffer &fb);
void processFrame(Framebuffer &fb);
} // namespace OS