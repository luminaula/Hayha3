#pragma once

extern "C" void* captureFrame(unsigned char *buffer,int x, int y, int width, int height);

extern "C" void captureLoop();
extern "C" void init(int width, int height);
extern "C" void deinit();
extern "C" void setMode(bool mode);
extern "C" void setTarget(char *target);
extern "C" void Error(void *e);
extern "C" bool isAttached();