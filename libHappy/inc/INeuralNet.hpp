#pragma once

#include <vector>
#include "bbox.hpp"
#include <string>
#include <math.h>

enum ColorFormat{
	NORM = 0,
	HFM = 2 //shifted cielab to make things hard for other people
};

struct image_t {
	int w,h; 				//dimensions of detection image
	int detectionX; 		//offset x
	int detectionY; 		//offset y
	int detectionH; 		//detection area h
	int detectionW; 		//detection area w
	int c; 					//num of channels
	float *data; 			//image data in rrggbb
	float thresh;			//threshold 0.0-1.0
	unsigned char *bdata;	//image data in rgb
	ColorFormat form;
};


void toHFM(image_t img);

void toNorm(image_t img);


struct NetCreateInfo{
	int width,height,gpu;
	char *config;
	char *weight;
	char *folder;
	char *precision;

	NetCreateInfo(){
		config = new char[0x4000];
		weight = new char[0x4000];
		folder = new char[0x4000];
		precision = new char[0x4000];
	}

	~NetCreateInfo(){
		//Memory leak
	}

};

struct TrainInfo{
	int width,height;
	int maxwidth,maxheight;
	int minwidth,minheight;
	char *config;
	char *weight;
	uint32_t iterations;
	std::vector<int> gpus;

};

class INeuralNet{
public:
	virtual ~INeuralNet(){}
	virtual std::vector<bbox_t> detect(image_t img, float thresh){
		bbox_t box;
		std::vector<bbox_t> boxes;
		box.x = img.detectionX;
		box.y = img.detectionY;
		box.w = img.detectionW;
		box.h = img.detectionH;
		box.prob = 1.0;
		box.obj_id = 0;
		boxes.push_back(box);
		return boxes;
	}
	virtual void loadClassifier(char *cfg, char *weight, int classes){}
	virtual int classify(image_t img){return 0;}
	virtual void train(int argc,char **argv){}
	virtual void train2(TrainInfo info){}
	virtual void setTrainData(std::vector<std::string> &files){}
};


