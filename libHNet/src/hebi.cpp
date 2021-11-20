#include "hebi.hpp"
#include "layer.h"

extern "C" {
#include "connected_layer.h"
#include "convolutional_layer.h"
#include "batchnorm_layer.h"
#include "blas.h"
#include "parser.h"

}



#include <iostream>
#include <fstream>
#include <sstream>


std::vector<box_label>  read_boxes_cust(char *filename, int *n){
    float x, y, h, w;
    int id;
    int count = 0;
    std::ifstream f(filename);
    std::string line;
    std::vector<box_label> boxs;
    while(getline(f,line)){
        if(line.length() < 2){
            break;
        }
        std::istringstream iss(line);
        box_label box;
        iss >> box.id >> box.x >> box.y >> box.w >> box.h;
        //boxes = (box_label*)realloc(boxes, (count+1)*sizeof(box_label));
        
        box.left   = box.x - box.w/2;
        box.right  = box.x + box.w/2;
        box.top    = box.y - box.h/2;
        box.bottom = box.y + box.h/2;
        ++count;
        boxs.push_back(box);
    }


    return boxs;

}


void transpose_matrix(float *a, int rows, int cols)
{
    float *transpose = (float*)calloc(rows*cols, sizeof(float));
    int x, y;
    for(x = 0; x < rows; ++x){
        for(y = 0; y < cols; ++y){
            transpose[y*rows + x] = a[x*cols + y];
        }
    }
    memcpy(a, transpose, rows*cols*sizeof(float));
    free(transpose);
}

void hwrite(void *src,int size,int count,char **fp){
    memcpy(*fp,src,count*size);
    *fp += count*size;
}

void hread(void *dst,int size,int count,char **fp){
    memcpy(dst,*fp,count*size);
    *fp += count*size;   
}

std::tuple<int,void*> get_convolutional_weights(layer l){

    int num = l.n*l.c*l.size*l.size;
    int totalSize = l.n + num;
    
    if(l.batch_normalize){
        totalSize += l.n * 3;
    }
    if(l.adam){
        totalSize += num * 2;
    }

    void *weights = malloc(totalSize * sizeof(float));

    char *weightptr = (char*)weights;

#ifdef GPU
    if(gpu_index >= 0){
        pull_convolutional_layer(l);
    }
#endif

    hwrite(l.biases,sizeof(float),l.n,&weightptr);

    if (l.batch_normalize){
        hwrite(l.scales,sizeof(float),l.n,&weightptr);
        hwrite(l.rolling_mean,sizeof(float),l.n,&weightptr);
        hwrite(l.rolling_variance,sizeof(float),l.n,&weightptr);
    }

    hwrite(l.weights,sizeof(float),num,&weightptr);

    if(l.adam){
        hwrite(l.m,sizeof(float),num,&weightptr);
        hwrite(l.v,sizeof(float),num,&weightptr);
    }

    return std::make_tuple(totalSize,weights);

}

void set_convolutional_weights(layer l,std::tuple<int,void*> weight){
    if(l.binary){
        //load_convolutional_weights_binary(l, fp);
        //return;
    }

    char *fp = (char*)std::get<1>(weight);

    int num = l.n*l.c*l.size*l.size;
    hread(l.biases, sizeof(float), l.n, &fp);
    if (l.batch_normalize && (!l.dontloadscales)){
        hread(l.scales, sizeof(float), l.n, &fp);
        hread(l.rolling_mean, sizeof(float), l.n, &fp);
        hread(l.rolling_variance, sizeof(float), l.n, &fp);
    }
    hread(l.weights, sizeof(float), num, &fp);
    if(l.adam){
        hread(l.m, sizeof(float), num, &fp);
        hread(l.v, sizeof(float), num, &fp);
    }
    //if(l.c == 3) scal_cpu(num, 1./256, l.weights, 1);
    if (l.flipped) {
        transpose_matrix(l.weights, l.c*l.size*l.size, l.n);
    }
    //if (l.binary) binarize_weights(l.weights, l.n, l.c*l.size*l.size, l.weights);
#ifdef GPU
    if(gpu_index >= 0){
        push_convolutional_layer(l);
    }
#endif
}

std::tuple<int,void*> get_connected_weights(layer l){

    int size = l.outputs + l.outputs*l.inputs;

    if(l.batch_normalize){
        size += l.outputs *3;
    }

    void *weights = malloc(size * sizeof(float));

    char *fp = (char*)weights;

#ifdef GPU
    if(gpu_index >= 0){
        pull_connected_layer(l);
    }
#endif
    hwrite(l.biases, sizeof(float), l.outputs, &fp);
    hwrite(l.weights, sizeof(float), l.outputs*l.inputs, &fp);
    if (l.batch_normalize){
        hwrite(l.scales, sizeof(float), l.outputs, &fp);
        hwrite(l.rolling_mean, sizeof(float), l.outputs, &fp);
        hwrite(l.rolling_variance, sizeof(float), l.outputs, &fp);
    }

    return std::make_tuple(size,weights);

}

std::tuple<int,void*> get_batchnorm_weights(layer l){

    int size = 3;

    void *weights = malloc(size * sizeof(float));

    char *fp = (char*)weights;

#ifdef GPU
    if(gpu_index >= 0){
        pull_batchnorm_layer(l);
    }
#endif
    hwrite(l.scales, sizeof(float), l.c, &fp);
    hwrite(l.rolling_mean, sizeof(float), l.c, &fp);
    hwrite(l.rolling_variance, sizeof(float), l.c, &fp);

    return std::make_tuple(size,weights);

}



std::vector<std::tuple<int,void*>> get_weights_upto(network net, int cutoff){
    std::vector<std::tuple<int,void*>> weights;

    for(int i=0;i<net.n;i++){
        layer l = net.layers[i];

        //TODO rest of the layers
        switch (l.type){
            case CONVOLUTIONAL:
                weights.push_back(get_convolutional_weights(l));
                break;
            case CONNECTED:
                weights.push_back(get_connected_weights(l));
                break;
            case BATCHNORM:
                weights.push_back(get_batchnorm_weights(l));
                break;
            case RNN:
                printf("rnn\n");
                break;
            case GRU:
                printf("gru\n");
                break;
            case CRNN:
                printf("crnn\n");
                break;
            default:

                break;
        }

    }

    return weights;

}

std::vector<std::tuple<int,void*>> get_weights(network net){
    return get_weights_upto(net,net.n);
}

void set_weights(network net, std::vector<std::tuple<int,void*>> weights){
    int windex = 0;
    for(int i=0;i<net.n;i++){
        layer l = net.layers[i];
        switch (l.type){
            case CONVOLUTIONAL:
                set_convolutional_weights(l,weights[windex++]);
                break;
            default:

                break;
        }

    }
}

void write_weights_mem(std::vector<std::tuple<int,void*>> &weights, std::string filename){
    FILE *fp = fopen(filename.c_str(),"wb");

    int major = 5414;
    int minor = 1;
    int revision = 0;
    fwrite(&major, sizeof(int), 1, fp);
    fwrite(&minor, sizeof(int), 1, fp);
    fwrite(&revision, sizeof(int), 1, fp);
    fwrite(&major, sizeof(int), 1, fp);



    for(auto &w : weights){
        fwrite((char*)std::get<1>(w),sizeof(float),std::get<0>(w),fp);
    }

    fclose(fp);
}