#include "clDetector.hpp"
#include "darknet.h"

ClDetector::ClDetector(char *cfg, char  *weight, int gpu_id){
	gpu_index = gpu_id;
	cl_set_device(gpu_id);
	net = load_network(cfg, weight, 0);
    set_batch_network(net, 1);
	net->gpu_index = gpu_id;

}

ClDetector::~ClDetector(){

}

std::vector<bbox_t> ClDetector::detect(image_t img, float thresh){
	cl_set_device(gpu_index);
	if(img.w != net->w || img.h != net->h)
		resize_network(net,img.w,img.h);
	
	layer l = net->layers[net->n - 1];
	
	float *X = img.data;
	float *prediction = network_predict(net, X);


	float hier_thresh = 0.5;
	int nboxes = 0;
	detection *dets = get_network_boxes(net, img.detectionW, img.detectionH, thresh, hier_thresh, 0, 1, &nboxes);

	if (nms) do_nms_sort(dets, nboxes, l.classes, nms);

	std::vector<bbox_t> bbox_vec;

	for (size_t i = 0; i < nboxes; ++i) {
		box b = dets[i].bbox;
		int const obj_id = max_index(dets[i].prob, l.classes);
		float const prob = dets[i].prob[obj_id];
		
		if (prob > thresh){
			bbox_t bbox;
			bbox.x = (std::max)((double)0, (b.x - b.w / 2.)*img.detectionW) + img.detectionX;
			bbox.y = (std::max)((double)0, (b.y - b.h / 2.)*img.detectionH) + img.detectionY;
			bbox.w = b.w*img.detectionW;
			bbox.h = b.h*img.detectionH;
			bbox.obj_id = obj_id;
			bbox.prob = prob;

			bbox_vec.push_back(bbox);
		}
	}


	free_detections(dets, nboxes);

	return bbox_vec;

}

extern "C" INeuralNet* createDetector(NetCreateInfo info){
	return new ClDetector(info.config,info.weight,info.gpu);
}