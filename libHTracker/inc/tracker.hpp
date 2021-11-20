#include "bbox.hpp"

extern "C" void initTracker(int x, int y, int width,int height,int numFrames);
extern "C" void setDimensions(int x, int y, int width,int height);
extern "C" std::vector<bbox_t> trackFrame(unsigned char *buffer);
extern "C" void resetTracker(unsigned char *buffer, std::vector<bbox_t> boxes);
