#include "yolo_v2_class.hpp"

#include "network.h"

extern "C" {
#include "detection_layer.h"
#include "region_layer.h"
#include "cost_layer.h"
#include "utils.h"
#include "parser.h"
#include "box.h"
#include "image.h"
#include "option_list.h"
#include "stb_image.h"
#include "data.h"
#include "hebic.h"
#include "image.h"

}
//#include <sys/time.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <thread>
#include "hcore.hpp"
#include "data.h"


#include "hebi.hpp"
#include "colorspaces.hpp"

#ifdef GPU
void check_cuda(cudaError_t status) {
	if (status != cudaSuccess) {
		const char *s = cudaGetErrorString(status);
		printf("CUDA Error Prev: %s\n", s);
	}
}
#endif

data load_data_detection_cust(int n, char **paths, int m, int w, int h, int c, int boxes, int classes, int use_flip, float jitter, float hue, float saturation, float exposure, int small_object)
{
    
    c = c ? c : 3;
    char **random_paths = get_random_paths(paths, n, m);
    int i;
    data d = { 0 };
    d.shallow = 0;

    d.X.rows = n;
    d.X.vals = (float**)calloc(d.X.rows, sizeof(float*));
    d.X.cols = h*w*c;

    d.y = make_matrix(n, 5 * boxes);
    for (i = 0; i < n; ++i) {
        image orig = load_image(random_paths[i], 0, 0, 3);

        int oh = orig.h;
        int ow = orig.w;

        int dw = (ow*jitter);
        int dh = (oh*jitter);

        int pleft = rand_uniform_strong(-dw, dw);
        int pright = rand_uniform_strong(-dw, dw);
        int ptop = rand_uniform_strong(-dh, dh);
        int pbot = rand_uniform_strong(-dh, dh);

        int swidth = ow - pleft - pright;
        int sheight = oh - ptop - pbot;

        float sx = (float)swidth / ow;
        float sy = (float)sheight / oh;

        int flip = use_flip ? random_gen() % 2 : 0;
        image cropped = crop_image(orig, pleft, ptop, swidth, sheight);

        float dx = ((float)pleft / ow) / sx;
        float dy = ((float)ptop / oh) / sy;

        image sized = resize_image(cropped, w, h);
        if (flip) flip_image(sized);

        random_distort_image(sized, hue, saturation, exposure);

        image ninechan = make_image(sized.w,sized.h,c);
        int fWidth = ninechan.w;
        int fHeight = ninechan.h;

        float *srcp[3];
        float *rgbp[3];
        float *xyzp[3];
        float *labp[3];

        srcp[0] = sized.data;
        srcp[1] = srcp[0] + fWidth * fHeight;
        srcp[2] = srcp[1] + fWidth * fHeight;

        rgbp[0] = ninechan.data;
        rgbp[1] = rgbp[0] + fWidth * fHeight;
        rgbp[2] = rgbp[1] + fWidth * fHeight;

        xyzp[0] = rgbp[2] + fWidth * fHeight;
        xyzp[1] = xyzp[0] + fWidth * fHeight;
        xyzp[2] = xyzp[1] + fWidth * fHeight;

        labp[0] = xyzp[2] + fWidth * fHeight;
        labp[1] = labp[0] + fWidth * fHeight;
        labp[2] = labp[1] + fWidth * fHeight;

        
        for(int k=0;k<ninechan.w*ninechan.h;k++){
            float r,g,b;
            r = *srcp[0]++ * 255.0;
            g = *srcp[1]++ * 255.0;
            b = *srcp[2]++ * 255.0;

            unsigned int color = ((unsigned char)r << 16) + ((unsigned char)g << 8) + ((unsigned char)b);

            HEBI::Colors::Color rgb = HEBI::Colors::rgb[color];
        
            *rgbp[0]++ = rgb.values[0];
            *rgbp[1]++ = rgb.values[1];
            *rgbp[2]++ = rgb.values[2];

            if(c > 3){
                HEBI::Colors::Color xyz = HEBI::Colors::xyz[color];
                *xyzp[0]++ = xyz.values[0];
                *xyzp[1]++ = xyz.values[1];
                *xyzp[2]++ = xyz.values[2];
            }
            if(c > 6){
                HEBI::Colors::Color lab = HEBI::Colors::lab[color];
                *labp[0]++ = lab.values[0];
                *labp[1]++ = lab.values[1];
                *labp[2]++ = lab.values[2];
            }
        }
        
        d.X.vals[i] = ninechan.data;

        fill_truth_detection(random_paths[i], boxes, d.y.vals[i], classes, flip, dx, dy, 1. / sx, 1. / sy, small_object, w, h);

        free_image(orig);
        free_image(cropped);
        free_image(sized);
    }
    free(random_paths);
    return d;
}

void load_thread_cust(void* ptr)
{
    //printf("Loading data: %d\n", rand());
    load_args a = *(struct load_args*)ptr;
    if(a.exposure == 0) a.exposure = 1;
    if(a.saturation == 0) a.saturation = 1;
    if(a.aspect == 0) a.aspect = 1;


    switch(a.type){
        case DETECTION_DATA:
            *a.d = load_data_detection_cust(a.n, a.paths, a.m, a.w, a.h, a.c, a.num_boxes, a.classes, a.flip, a.jitter, a.hue, a.saturation, a.exposure, a.small_object);
            break;
        case CLASSIFICATION_DATA:
            *a.d = load_data_augment(a.paths, a.n, a.m, a.labels, a.classes, a.hierarchy, a.flip, a.min, a.max, a.size, a.angle, a.aspect, a.hue, a.saturation, a.exposure);
            break;
        default:
            break;
    }
    free(ptr);
}



std::thread load_data_in_thread_cust(load_args args)
{
    struct load_args *ptr = (load_args *)calloc(1, sizeof(struct load_args));
    *ptr = args;
    return std::thread(load_thread_cust,(void*)ptr);
}

void load_threads_cust(void* ptr)
{
    
    int i;
    load_args args = *(load_args *)ptr;
    if (args.threads == 0) args.threads = 1;
    data *out = args.d;
    int total = args.n;
    free(ptr);
    data *buffers = (data*)calloc(args.threads, sizeof(data));
	std::thread* threads = new std::thread[args.threads];
    for(i = 0; i < args.threads; ++i){
        args.d = buffers + i;
        args.n = (i+1) * total/args.threads - i * total/args.threads;
        threads[i] = load_data_in_thread_cust(args);
    }
    for(i = 0; i < args.threads; ++i){
		threads[i].join();
    }
    *out = concat_datas(buffers, args.threads);
    out->shallow = 0;
    for(i = 0; i < args.threads; ++i){
        buffers[i].shallow = 1;
        free_data(buffers[i]);
    }
    free(buffers);
	delete[] threads;
}


std::thread load_data_cust(load_args args)
{
    struct load_args *ptr = (load_args *)calloc(1, sizeof(struct load_args));
    *ptr = args;
	return std::thread(load_threads_cust, (void*)ptr);
}


struct detector_gpu_t {
	network net;
};

struct classifier_gpu_t{
    network net;
};

Detector::Detector(char *cfg, char  *weight, int gpu_id) : cur_gpu_id(gpu_id)
{
	wait_stream = 0;
	int old_gpu_index;
#ifdef GPU
	check_cuda( cudaGetDevice(&old_gpu_index) );
#endif

	detector_gpu_ptr = std::make_shared<detector_gpu_t>();
	detector_gpu_t &detector_gpu = *static_cast<detector_gpu_t *>(detector_gpu_ptr.get());

#ifdef GPU
	//check_cuda( cudaSetDevice(cur_gpu_id) );
	cuda_set_device(cur_gpu_id);
	//printf(" Used GPU %d \n", cur_gpu_id);
#endif
	network &net = detector_gpu.net;
	net.gpu_index = cur_gpu_id;
	//gpu_index = i;
	


	net = parse_network_cfg_custom(cfg, 1);
	if (weight) {
		load_weights(&net, weight);
	}
	set_batch_network(&net, 1);
	net.gpu_index = cur_gpu_id;
	fuse_conv_batchnorm(net);

	layer l = net.layers[net.n - 1];
	int j;


    //auto w = get_weights(net);
    

    //free_network(net);

    //net = parse_network_cfg_custom(cfg, 1);

    //set_weights(net,w);

#ifdef GPU
	check_cuda( cudaSetDevice(old_gpu_index) );
#endif

    

}

Detector::Detector() : cur_gpu_id(0),m_trainImages(0),m_numTrainImages(0){

}


Detector::~Detector() 
{
	detector_gpu_t &detector_gpu = *static_cast<detector_gpu_t *>(detector_gpu_ptr.get());
	layer l = detector_gpu.net.layers[detector_gpu.net.n - 1];

	int old_gpu_index;
#ifdef GPU
	cudaGetDevice(&old_gpu_index);
	cuda_set_device(detector_gpu.net.gpu_index);
#endif

	free_network(detector_gpu.net);

#ifdef GPU
	cudaSetDevice(old_gpu_index);
#endif
}




std::vector<bbox_t> Detector::detect(image_t img, float thresh)
{
	detector_gpu_t &detector_gpu = *static_cast<detector_gpu_t *>(detector_gpu_ptr.get());
	network &net = detector_gpu.net;
	int old_gpu_index;
    image im;
    im.data = img.data;
    im.w = img.w;
    im.h = img.h;
    im.c = 3;
    //Convert color space to make it harder for others to steal my shit
    //rgb_to_hfm(im);
#ifdef GPU
	cudaGetDevice(&old_gpu_index);
	if(cur_gpu_id != old_gpu_index)
		cudaSetDevice(net.gpu_index);

	net.wait_stream = wait_stream;	// 1 - wait CUDA-stream, 0 - not to wait
#endif

	if(img.w != net.w || img.h != net.h)
		resize_network(&net,img.w,img.h);


	layer l = net.layers[net.n - 1];
	float *X = img.data;
	float *prediction = network_predict(net, X);

	int nboxes = 0;
	int letterbox = 0;
	float hier_thresh = 0.5;
	detection *dets = get_network_boxes(&net, img.detectionW, img.detectionH, thresh, hier_thresh, 0, 1, &nboxes, letterbox);
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


#ifdef GPU
	if (cur_gpu_id != old_gpu_index)
		cudaSetDevice(old_gpu_index);
#endif

	return bbox_vec;
}

void Detector::train_real(char *datacfg, char *cfgfile, char *weightfile, int *gpus, int ngpus, int clear, int dont_show){
   
}

void Detector::train(int argc,char **argv){


}

void Detector::train2(TrainInfo info){
    HEBI::Colors::init();
    #ifdef GPU
    cudaSetDevice(cur_gpu_id);
    #endif

    int ngpus = 1;
    int count = 0;
    //network *nets = (network*)calloc(1, sizeof(network));

    network *nets;


    if(!m_netTrain){
        m_netTrain = calloc(1,sizeof(network));
        nets = (network*)m_netTrain;
        nets[0] = parse_network_cfg(info.config);
        load_weights(&nets[0],info.weight);
    }


    

    

    nets = (network*)m_netTrain;
    network net = nets[0];

    //

    //network net = nets[0];

    const int actual_batch_size = net.batch * net.subdivisions;
    if (actual_batch_size == 1) {
        printf("\n Error: You set incorrect value batch=1 for Training! You should set batch=64 subdivision=64 \n");
        getchar();
    }
    else if (actual_batch_size < 64) {
            printf("\n Warning: You set batch=%d lower than 64! It is recommended to set batch=64 subdivision=64 \n", actual_batch_size);
    }

    int imgs = net.batch * net.subdivisions;
    
    data train, buffer;

    layer l = net.layers[net.n - 1];

    int classes = l.classes;
    float jitter = 0;


    int init_w = info.width;
    int init_h = info.height;

    load_args args = {0};
    args.w = net.w;
    args.h = net.h;
    args.c = net.c;
    args.paths = m_trainImages;
    args.n = imgs;
    args.m = m_numTrainImages;
    args.classes = classes;
    args.flip = net.flip;
    args.jitter = jitter;
    args.num_boxes = l.max_boxes;
    args.small_object = net.small_object;
    args.d = &buffer;
    args.type = DETECTION_DATA;
    args.threads = 64;

    args.angle = net.angle;
    args.exposure = net.exposure;
    args.saturation = net.saturation;
    args.hue = net.hue;

    std::thread load_thread = load_data_cust(args);
	
    timeStamp timee = getCurrentTimeMicro();

    args.w = info.width;
    args.h = info.height;

    load_thread.join();
    train = buffer;
    free_data(train);
    load_thread = load_data_cust(args);

    
    resize_network(nets, args.w, args.h);
    
    net = nets[0];
    

    std::vector<float> losses;

    for(int i=0;i<info.iterations;i++){

        
		load_thread.join();
        train = buffer;
        load_thread = load_data_cust(args);
        float loss = 0;


        loss = train_network(net, train);

        losses.push_back(loss);

        //printf("%f\n",loss);


        free_data(train);

        

    }

    int64_t micros = timeSince(timee);


    float avg_loss= 0.0;
    float total_loss = 0.0;
    for(auto &losss: losses ){
        total_loss += losss; 
    }

    avg_loss = total_loss/losses.size();


    printf("%d: %d iterations in %f seconds, loss %f\n",get_current_batch(net), info.iterations,(float)micros/1000000.0,avg_loss);

    printf("Learning Rate: %f, Momentum: %f, Decay: %f\n", get_current_rate(net), net.momentum, net.decay);
    
    float avgTime = (float)micros/1000000.0 / info.iterations;

    int batchesLeft = net.max_batches - get_current_batch(net);

    int timeLeft = avgTime * batchesLeft;

    int hoursLeft = floor(timeLeft / 3600);
    int minutesLeft = timeLeft % 3600 / 60;
    int secondsLeft = timeLeft % 60;


    printf("Time to complete %d:%02d:%02d\n",hoursLeft,minutesLeft,secondsLeft);
    

    save_weights(net,info.weight);

    load_thread.join();
    free_data(buffer);

    //free_network(net);
}

void Detector::setTrainData(std::vector<std::string> &files){
    if(m_trainImages){
        for(int i=0;i<m_numTrainImages;i++){
            free(m_trainImages[i]);
        }
        free(m_trainImages);
    }
    m_numTrainImages = 0;
    m_trainImages = (char **)calloc(files.size(),sizeof(char*));
    for(auto &file :files){
        m_trainImages[m_numTrainImages] = (char*)calloc(file.size()+1,sizeof(char));
        sprintf(m_trainImages[m_numTrainImages],"%s",file.c_str());
        m_numTrainImages++;
    }
}

void Detector::loadClassifier(char *cfg, char *weight,int classes){
    classifier_gpu_ptr = std::make_shared<detector_gpu_t>();
    detector_gpu_t &detector_gpu = *static_cast<detector_gpu_t *>(classifier_gpu_ptr.get());
    network &classifierNet = detector_gpu.net;
    classifierNet = parse_network_cfg_custom(cfg,1);
    if(weight)
        load_weights(&classifierNet,weight);
    set_batch_network(&classifierNet,1);
    
    
}

int Detector::classify(image_t img){
    
    detector_gpu_t &classifier_gpu = *static_cast<detector_gpu_t *>(classifier_gpu_ptr.get());
    network &classifierNet =  classifier_gpu.net;

    float *predictions = network_predict(classifierNet,img.data);
    //if(classifierNet.hierarchy) hierarchy_predictions(predictions,classifierNet.outputs,classifierNet.hierarchy,0);
    int index = 0;

    top_k(predictions,classifierNet.outputs,1,&index);

    printf("%f\n",predictions[index]);

    return index;
}



YOLODLL_API INeuralNet* createDetector(NetCreateInfo info){
    //Remove const from string
	return new Detector(info.config,info.weight,info.gpu);
}

YOLODLL_API INeuralNet* createTrainer(){
    HEBI::Colors::init();
	return new Detector();
}