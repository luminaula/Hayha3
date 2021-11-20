#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <dirent.h>
#include <string.h>
#include <string>
#include <thread>

#include "hcore.hpp"

HCore::HCore *core;
HCore::HCore *auxCore;

using namespace cv;

int findXmin(Mat &image){
    for(int x=0;x<image.cols;x++){
        for(int y=0;y<image.rows;y++){
            uint32_t alpha = image.at<Vec4b>(y,x)[3];
            if(alpha == 0xff)
                return x;
        }
    }
}

int findYmin(Mat &image){
    for(int y=0;y<image.rows;y++){
        for(int x=0;x<image.cols;x++){
            uint32_t alpha = image.at<Vec4b>(y,x)[3];
            if(alpha == 0xff)
                return y;
        }
    }
}

int findXmax(Mat &image){
    for(int i=image.cols-1;i>0;i--){
        for(int j=0;j<image.rows;j++){
            uint32_t alpha = image.at<Vec4b>(j,i)[3];
            if(alpha == 0xff)
                return i;
        }
    }
}

int findYmax(Mat &image){
    for(int i=image.rows-1;i>0;i--){
        for(int j=0;j<image.cols;j++){
            uint32_t alpha = image.at<Vec4b>(i,j)[3];
            if(alpha == 0xff)
                return i;
        }
    }
}

void processImage(std::string srcPath,std::string destPath){

    int xmi,ymi,xma,yma;

    Mat rgbaImage = imread( srcPath.c_str(), IMREAD_UNCHANGED  );

    auto xmin = auxCore->tPool->enqueue(findXmin,rgbaImage);
    auto ymin = auxCore->tPool->enqueue(findYmin,rgbaImage);
    auto xmax = auxCore->tPool->enqueue(findXmax,rgbaImage);
    auto ymax = auxCore->tPool->enqueue(findYmax,rgbaImage);
    
    xmin.wait();
    ymin.wait();
    xmax.wait();
    ymax.wait();

    xmi = xmin.get();
    xma = xmax.get();
    ymi = ymin.get();
    yma = ymax.get();

    printf("%d %d %d %d \n",xmi,ymi,xma,yma);

    try{
        Rect ROI(xmi,ymi,xma-xmi,yma-ymi);

        Mat cropped = rgbaImage(ROI);
        
        imwrite(destPath.c_str(),cropped);
    }
    catch(...){

    }
}

void cropImagesInFolder(char *src, char *dst){
    DIR *dir;
    dirent *ent;
    dir = opendir(src);
    

    std::string filepath;
    std::string destFilepath;
    uint16_t key = core->generateThreadKey();
    if(dir != NULL){
        while(ent = readdir(dir)){
            if(strlen(ent->d_name) > 4){
                //printf("File %s\n",ent->d_name);
                filepath.clear();
                filepath = std::string(src) + std::string("/") + std::string(ent->d_name);
                destFilepath = std::string(dst) + std::string("/") + std::string(ent->d_name);
                std::cout << filepath << std::endl;

                core->tPool->enqueueBlocking(key,processImage,filepath,destFilepath);

                
            }
        }
        core->tPool->waitForTasks(key,0);
        closedir(dir);
    }
}

int main(int argc, char** argv ){

    if(argc < 3)
        return 1;

    core = HCore::init(16,1);
    auxCore = new HCore::HCore(16,1);

    cropImagesInFolder(argv[1],argv[2]);
    //cropImagesInFolder("../Players/tm_leet/images","../Players/tm_leet/cropped");



    return 0;
}