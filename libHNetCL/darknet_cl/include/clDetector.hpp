#pragma once

#include <memory>
#include <vector>
#include <deque>
#include <algorithm>

#include "INeuralNet.hpp"
#include "darknet.h"

class ClDetector : public INeuralNet{
private:
	network *net;
public:
	float nms = .4;

	ClDetector(char *cfg, char  *weight, int gpu_id);
	~ClDetector();

	std::vector<bbox_t> detect(image_t img, float thresh);

};

extern "C" INeuralNet* createDetector(NetCreateInfo info);

