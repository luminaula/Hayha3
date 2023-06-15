#pragma once

#ifdef YOLODLL_EXPORTS
#if defined(_MSC_VER)
#define YOLODLL_API __declspec(dllexport)
#else
#define YOLODLL_API __attribute__((visibility("default")))
#endif
#else
#if defined(_MSC_VER)
#define YOLODLL_API __declspec(dllimport)
#else
#define YOLODLL_API
#endif
#endif


#ifdef __cplusplus
#include <memory>
#include <vector>
#include <deque>
#include <algorithm>

#include "INeuralNet.hpp"


class Detector : public INeuralNet{
	std::shared_ptr<void> detector_gpu_ptr;
	std::shared_ptr<void> classifier_gpu_ptr;
	//network classifierNet;
	const int cur_gpu_id;
public:
	float nms = .4;
	bool wait_stream;

	Detector(char *cfg, char  *weight, int gpu_id);
	Detector();
	~Detector();

	std::vector<bbox_t> detect(image_t img, float thresh) override;
	void train(int argc,char **argv) override;
	void train_real(char *datacfg, char *cfgfile, char *weightfile, int *gpus, int ngpus, int clear, int dont_show);

	void loadClassifier(char *cfg, char *weight, int classes) override;
	int classify(image_t img) override;

};

extern "C" YOLODLL_API INeuralNet* createDetector(NetCreateInfo info);
extern "C" YOLODLL_API INeuralNet* createTrainer();

#endif
