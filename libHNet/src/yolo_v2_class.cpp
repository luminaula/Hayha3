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
}
//#include <sys/time.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <thread>
#include "hcore.hpp"

#ifdef GPU
void check_cuda(cudaError_t status) {
	if (status != cudaSuccess) {
		const char *s = cudaGetErrorString(status);
		printf("CUDA Error Prev: %s\n", s);
	}
}
#endif


void load_thread_cust(void* ptr)
{
    //printf("Loading data: %d\n", rand());
    load_args a = *(struct load_args*)ptr;
    if(a.exposure == 0) a.exposure = 1;
    if(a.saturation == 0) a.saturation = 1;
    if(a.aspect == 0) a.aspect = 1;


    switch(a.type){
        case DETECTION_DATA:
            *a.d = load_data_detection(a.n, a.paths, a.m, a.w, a.h, a.c, a.num_boxes, a.classes, a.flip, a.jitter, a.hue, a.saturation, a.exposure, a.small_object);
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


#ifdef GPU
	check_cuda( cudaSetDevice(old_gpu_index) );
#endif
}

Detector::Detector() : cur_gpu_id(0){

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
    list *options = read_data_cfg(datacfg);
    char *train_images = option_find_str(options, "train", "data/train.list");
    char *backup_directory = option_find_str(options, "backup", "/backup/");

    srand(time(0));
    char *base = basecfg(cfgfile);
    printf("%s\n", base);
    float avg_loss = -1;
    network *nets = (network*)calloc(ngpus, sizeof(network));

    srand(time(0));
    int seed = rand();
    int i;
    for(i = 0; i < ngpus; ++i){
        srand(seed);
#ifdef GPU
        cuda_set_device(gpus[i]);
#endif
        nets[i] = parse_network_cfg(cfgfile);
        if(weightfile){
            load_weights(&nets[i], weightfile);
        }
        if(clear) *nets[i].seen = 0;
        nets[i].learning_rate *= ngpus;
    }
    srand(time(0));
    network net = nets[0];

    const int actual_batch_size = net.batch * net.subdivisions;
    if (actual_batch_size == 1) {
        printf("\n Error: You set incorrect value batch=1 for Training! You should set batch=64 subdivision=64 \n");
        getchar();
    }
    else if (actual_batch_size < 64) {
            printf("\n Warning: You set batch=%d lower than 64! It is recommended to set batch=64 subdivision=64 \n", actual_batch_size);
    }

    int imgs = net.batch * net.subdivisions * ngpus;
    printf("Learning Rate: %g, Momentum: %g, Decay: %g\n", net.learning_rate, net.momentum, net.decay);
    data train, buffer;

    layer l = net.layers[net.n - 1];

    int classes = l.classes;
    float jitter = l.jitter;

    list *plist = get_paths(train_images);
    //int N = plist->size;
    char **paths = (char **)list_to_array(plist);

    int init_w = net.w;
    int init_h = net.h;
    int iter_save;
    int iter_save2 = get_current_batch(net);
    iter_save = get_current_batch(net);

    load_args args = {0};
    args.w = net.w;
    args.h = net.h;
    args.c = net.c;
    args.paths = paths;
    args.n = imgs;
    args.m = plist->size;
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
	timeStamp time = getCurrentTimeMicro();

    int count = 0;

    while(get_current_batch(net) < net.max_batches){
        if(l.random && count++%10 == 0){
            //HCore::core->log(LOG_INFO,"Resizing");
            //int dim = (rand() % 12 + (init_w/32 - 5)) * 32;    // +-160
            //int dim = (rand() % 4 + 16) * 32;
            //if (get_current_batch(net)+100 > net.max_batches) dim = 544;

            //int random_val = rand() % 12;
            //int dim_w = (random_val + (init_w / 32 - 5)) * 32;    // +-160
            //int dim_h = (random_val + (init_h / 32 - 5)) * 32;    // +-160

            float random_val = rand_scale(1.4);    // *x or /x
            int dim_w = roundl(random_val*init_w / 32) * 32;
            int dim_h = roundl(random_val*init_h / 32) * 32;

            if (dim_w < 32) dim_w = 32;
            if (dim_h < 32) dim_h = 32;

            //HCore::core->log(LOG_INFO,"%d x %d ", dim_w, dim_h);
            printf("resize %d x %d \n", dim_w, dim_h);
            args.w = dim_w;
            args.h = dim_h;

			load_thread.join();
            train = buffer;
            free_data(train);
            load_thread = load_data_cust(args);

            for(i = 0; i < ngpus; ++i){
                resize_network(nets + i, dim_w, dim_h);
            }
            net = nets[0];
        }
        
        time = getCurrentTimeMicro();
		load_thread.join();
        train = buffer;
        load_thread = load_data_cust(args);

        /*
           int k;
           for(k = 0; k < l.max_boxes; ++k){
           box b = float_to_box(train.y.vals[10] + 1 + k*5);
           if(!b.x) break;
           printf("loaded: %f %f %f %f\n", b.x, b.y, b.w, b.h);
           }
         */
        /*
           int zz;
           for(zz = 0; zz < train.X.cols; ++zz){
           image im = float_to_image(net->w, net->h, 3, train.X.vals[zz]);
           int k;
           for(k = 0; k < l.max_boxes; ++k){
           box b = float_to_box(train.y.vals[zz] + k*5, 1);
           printf("%f %f %f %f\n", b.x, b.y, b.w, b.h);
           draw_bbox(im, b, 1, 1,0,0);
           }
           show_image(im, "truth11");
           cvWaitKey(0);
           save_image(im, "truth11");
           }
         */
        //HCore::core->log(LOG_INFO,"Loaded: %lf seconds",(float)timeSince(time)/1000000.0);
        printf("Loaded: %lf seconds\n",(float)timeSince(time)/1000000.0);
        time = getCurrentTimeMicro();
        float loss = 0;
#ifdef GPU
        if(ngpus == 1){
            loss = train_network(net, train);
        } else {
            loss = train_networks(nets, ngpus, train, 4);
        }
#else
        loss = train_network(net, train);
#endif
        if (avg_loss < 0 || avg_loss != avg_loss) avg_loss = loss;    // if(-inf or nan)
        avg_loss = avg_loss*.9 + loss*.1;

        i = get_current_batch(net);
        //HCore::core->log(LOG_INFO,"%d: %f, %f avg loss, %f rate, %lf seconds, %d images", get_current_batch(net), loss, avg_loss, get_current_rate(net), (float)timeSince(time)/1000000.0, i*imgs);
        printf("%d: %f, %f avg loss, %f rate, %lf seconds, %d images\n", get_current_batch(net), loss, avg_loss, get_current_rate(net), (float)timeSince(time)/1000000.0, i*imgs);

        //if (i % 1000 == 0 || (i < 1000 && i % 100 == 0)) {
        //if (i % 100 == 0) {
        if(i >= (iter_save + 100)) {
            iter_save = i;
#ifdef GPU
            if (ngpus != 1) sync_nets(nets, ngpus, 0);
#endif
            char buff[256];
            sprintf(buff, "%s/backup.weight", backup_directory, base, i);
            save_weights(net, buff);
        }
        free_data(train);
    }
#ifdef GPU
    if(ngpus != 1) sync_nets(nets, ngpus, 0);
#endif
    char buff[256];
    sprintf(buff, "%s/%s_%s.weights", backup_directory, base,i);
    save_weights(net, buff);



    // free memory
    load_thread.join();
    free_data(buffer);

    free(base);
    free(paths);
    free_list_contents(plist);
    free_list(plist);

    free_list_contents_kvp(options);
    free_list(options);

    free(nets);
    free_network(net);
}

void Detector::train(int argc,char **argv){
	int dont_show = find_arg(argc, argv, "-dont_show");
    int show = find_arg(argc, argv, "-show");
    int http_stream_port = find_int_arg(argc, argv, "-http_port", -1);
    char *out_filename = find_char_arg(argc, argv, "-out_filename", 0);
    char *outfile = find_char_arg(argc, argv, "-out", 0);
    char *prefix = find_char_arg(argc, argv, "-prefix", 0);
    float thresh = find_float_arg(argc, argv, "-thresh", .25);    // 0.24
    float hier_thresh = find_float_arg(argc, argv, "-hier", .5);
    int cam_index = find_int_arg(argc, argv, "-c", 0);
    int frame_skip = find_int_arg(argc, argv, "-s", 0);
    int num_of_clusters = find_int_arg(argc, argv, "-num_of_clusters", 5);
    int width = find_int_arg(argc, argv, "-width", -1);
    int height = find_int_arg(argc, argv, "-height", -1);
    // extended output in test mode (output of rect bound coords)
    // and for recall mode (extended output table-like format with results for best_class fit)
    int ext_output = find_arg(argc, argv, "-ext_output");
    int save_labels = find_arg(argc, argv, "-save_labels");
    if(argc < 4){
        fprintf(stderr, "usage: %s [data] [cfg] [weights (optional)]\n", argv[0]);
        return;
    }
    char *gpu_list = find_char_arg(argc, argv, "-gpus", 0);
    int *gpus = 0;
    int gpu = 0;
    int ngpus = 0;
    if(gpu_list){
        printf("%s\n", gpu_list);
        int len = strlen(gpu_list);
        ngpus = 1;
        int i;
        for(i = 0; i < len; ++i){
            if (gpu_list[i] == ',') ++ngpus;
        }
        gpus = (int*)calloc(ngpus, sizeof(int));
        for(i = 0; i < ngpus; ++i){
            gpus[i] = atoi(gpu_list);
            gpu_list = strchr(gpu_list, ',')+1;
        }
    } else {
        gpu = cur_gpu_id;
        gpus = &gpu;
        ngpus = 1;
    }

    int clear = find_arg(argc, argv, "-clear");

    char *datacfg = argv[1];
    char *cfg = argv[2];
    char *weights = (argc > 3) ? argv[3] : 0;
    if(weights)
        if(strlen(weights) > 0)
            if (weights[strlen(weights) - 1] == 0x0d) weights[strlen(weights) - 1] = 0;
    train_real(datacfg, cfg, weights, gpus, ngpus, clear, dont_show);

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
	return new Detector();
}