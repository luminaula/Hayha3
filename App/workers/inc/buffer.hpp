#ifndef BUFFER_HPP
#define BUFFER_HPP

#include "framebuffer.hpp"

#define FRAMEBUFFER_COUNT 64

namespace HBuffer {

enum Stage_t { CAPTURE, DETECT, PRESENT };

extern std::vector<Framebuffer> framebuffers;

void init(uint32_t count);

void resize(int index);
Framebuffer &getFramebuffer(uint32_t index);
Framebuffer &getFramebuffer(Stage_t stage);
void nextFramebuffer(Stage_t stage);

void finishCapture(int id);
void finishDetect(int id);

} // namespace HBuffer

#endif