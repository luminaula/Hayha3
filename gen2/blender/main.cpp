#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <string.h>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>

#include "hcore.hpp"

using namespace cv;

#define AMOUNT 25600
#define MIN_OBJS_PER_IMAGE 1
#define OBJS_PER_IMG 4

std::vector<std::string> backGrounds;
std::vector<std::string> foreGrounds;
std::vector<std::string> masks;
std::vector<std::string> foreGroundClassName;

std::string destinationImgFolder = "/media/xoxo/2f03a838-cd91-4e2b-bfbb-968314f77511/chicken/final/images/";
std::string destinationAnnFolder = "/media/xoxo/2f03a838-cd91-4e2b-bfbb-968314f77511/chicken/final/annotations/";

int minHead = 5 * 5;

HCore::HCore *core;
int tries = 3;

struct ObjectAnnotation{
    int xmin,ymin,xmax,ymax;
    std::string name;
};

struct Annotation{
    std::string imgName;
    std::string annotationName;
    std::vector<ObjectAnnotation> objects;
    std::vector<ObjectAnnotation> heads;
    int w,h;
};

float inline lerp(float s, float e, float t){
    return s+(e-s)*t;
}

bool hasHead(Mat &mask){
    int headAmount = 0;
    for(int i=0;i<mask.rows;i++){
        unsigned int *ptr = mask.ptr<unsigned int>(i);
        for(int j=0;j<mask.cols;j++){
            if(ptr[j] == 0xFFFF0000){
                headAmount++;
            }
        }
    }
    if(headAmount >= minHead)
        return true;
    else
        return false;
}


void aBlend(Mat &src, Mat &dest, int x,int y, int w, int h){
    int srcx = 0;
    int srcy = 0;
    for(int i=x;i<x+w;i++){
        for(int j=y;j<y+h;j++){
            Vec4b asd = src.at<Vec4b>(srcy,srcx);
            uint8_t srcb = (uint8_t)asd[0];
            uint8_t srcg = (uint8_t)asd[1];
            uint8_t srcr = (uint8_t)asd[2];
            uint8_t srca = (uint8_t)asd[3];

            if((asd[3] & 0xff) == 0xff){
                Vec3b destVec;
                destVec[0] = asd[0];
                destVec[1] = asd[1];
                destVec[2] = asd[2];
                dest.at<Vec3b>(j,i) = destVec;
            }

            
            srcy++;
        }
        srcx++;
        srcy=0;
    }

}

void saveImage(Mat &src, Annotation &ann){
    std::string imageFilePath = destinationImgFolder + ann.imgName;
    imwrite(imageFilePath.c_str(),src);
}

void createAnnotationFile(Annotation &ann){
    std::string annotationFilePath = destinationAnnFolder + ann.annotationName;
    std::ofstream annotationFile;
    annotationFile.open(annotationFilePath.c_str());
    annotationFile << "<annotation>" << std::endl; 
    annotationFile << "<imageName>" << ann.imgName << "</imageName>" << std::endl;
    annotationFile << "<width>" << ann.w << "</width>" << std::endl;
    annotationFile << "<height>" << ann.h << "</height>" << std::endl;
    for( auto &obj : ann.objects){
        annotationFile << "<object>" << std::endl;
        annotationFile << "\t<objName>" << obj.name << "</objName>" << std::endl;
        annotationFile << "\t<xmin>" << obj.xmin << "</xmin>" << std::endl;
        annotationFile << "\t<xmax>" << obj.xmax << "</xmax>" << std::endl;
        annotationFile << "\t<ymin>" << obj.ymin << "</ymin>" << std::endl;
        annotationFile << "\t<ymax>" << obj.ymax << "</ymax>" << std::endl;
        annotationFile << "</object>" << std::endl;
    }
    for( auto &obj : ann.heads){
        annotationFile << "<object>" << std::endl;
        annotationFile << "\t<objName>" << obj.name << "</objName>" << std::endl;
        annotationFile << "\t<xmin>" << obj.xmin << "</xmin>" << std::endl;
        annotationFile << "\t<xmax>" << obj.xmax << "</xmax>" << std::endl;
        annotationFile << "\t<ymin>" << obj.ymin << "</ymin>" << std::endl;
        annotationFile << "\t<ymax>" << obj.ymax << "</ymax>" << std::endl;
        annotationFile << "</object>" << std::endl;
    }
    annotationFile << "</annotation>" << std::endl;
    annotationFile.close(); 
}

float randFloat(float min,float max){
    float scale = rand() / (float) RAND_MAX;
    return min + scale * ( max - min );
}

ObjectAnnotation getHeadAnnotation(Mat &mask){
    ObjectAnnotation head;
    for(int i=0;i<mask.rows;i++){
        unsigned int *ptr = mask.ptr<unsigned int>(i);
        for(int j=0;j<mask.cols;j++){
            if(ptr[j] == 0xFFFF0000){
                head.ymin = i;
                goto end1;
            }
        }
    }
    end1:
    for(int i=mask.rows-1; i>= 0;i--){
        unsigned int *ptr = mask.ptr<unsigned int>(i);
        for(int j=0;j<mask.cols;j++){
            if(ptr[j] == 0xFFFF0000){
                head.ymax = i;
                goto end2;
            }
        }
    }
    end2:
    for(int i=0;i<mask.cols;i++){
        for(int j=0;j<mask.rows;j++){
            unsigned int pix = mask.at<unsigned int>(j,i);
            if(pix == 0xFFFF0000){
                head.xmin = i;
                goto end3;
            }
        }
    }
    end3:

    for(int i=mask.cols-1;i>=0;i--){
        for(int j=0;j<mask.rows;j++){
            unsigned int pix = mask.at<unsigned int>(j,i);
            if(pix == 0xFFFF0000){
                head.xmax = i;
                goto end4;
            }
        }
    }
    end4:

    head.name = "head";
    return head;
}

void createImage(uint32_t count){
    std::cout << count << std::endl;
    uint32_t destIndex = rand() % backGrounds.size();
    uint32_t objs = rand() % (OBJS_PER_IMG - MIN_OBJS_PER_IMAGE) + MIN_OBJS_PER_IMAGE;
    Mat destt = imread(backGrounds.at(destIndex));
    Mat dest;
    float destSize = randFloat(0.7,1.0);
    resize(destt,dest,Size(),destSize,destSize);
    //std::cout << "Background = " << backGrounds.at(destIndex) << ", ObjCount: " << objs << std::endl;
    uint32_t width = dest.cols;
    uint32_t height = dest.rows;
    uint32_t widthUsed = 0;
    Annotation annotations;
    annotations.imgName = std::to_string(count) + std::string(".jpg");
    annotations.annotationName = std::to_string(count) + std::string(".xml");
    annotations.w = dest.cols;
    annotations.h = dest.rows;
    int i = 0;
    int tried = 0;
    do{
        if(widthUsed >= dest.cols * 0.8)
            goto end;
        ObjectAnnotation ann = {0};
        uint32_t srcIndex = rand() % foreGrounds.size();
        Mat srcc = imread(foreGrounds.at(srcIndex), IMREAD_UNCHANGED );
        Mat maskk = imread(masks.at(srcIndex), IMREAD_UNCHANGED);
        if(srcc.cols > srcc.rows)
            continue;
        Mat src;
        Mat mask;
        float resizef = randFloat(0.2,1.0);
        resize(srcc,src,Size(),resizef,resizef);
        resize(maskk,mask,Size(),resizef,resizef);
        if(src.rows * src.cols < 200){
            continue;
        }
        if(src.rows >= dest.rows || src.cols >= dest.cols)
            continue;
        ann.xmin = widthUsed;
        ann.ymin = rand() % (height-src.rows);
        ann.ymax = std::min(ann.ymin+src.rows,dest.rows-1);
        ann.xmax = std::min(ann.xmin+src.cols,dest.cols-1);
        ann.name = foreGroundClassName.at(srcIndex);

        if(ann.xmax == dest.cols-1)
            continue;

        //std::cout << "\tForeground = " << foreGrounds.at(srcIndex) << " Type " << foreGroundClassName.at(srcIndex) << std::endl;
        //std::cout << "\t\txmin " << ann.xmin << " ymin " << ann.ymin << " xmax " << ann.xmax << " ymax " << ann.ymax << std::endl;
        aBlend(src,dest,ann.xmin,ann.ymin,ann.xmax-ann.xmin,ann.ymax-ann.ymin);

        annotations.objects.push_back(ann);

        if(hasHead(mask)){
            ObjectAnnotation headAnn = getHeadAnnotation(mask);
            headAnn.xmin = std::min(ann.xmin + headAnn.xmin,dest.cols);
            headAnn.xmax = std::min(ann.xmin + headAnn.xmax,dest.cols);
            headAnn.ymin = std::min(ann.ymin + headAnn.ymin,dest.rows);
            headAnn.ymax = std::min(ann.ymin + headAnn.ymax,dest.rows);
            headAnn.name = foreGroundClassName.at(srcIndex) + "head";
            if(!(headAnn.xmin == headAnn.xmax || headAnn.ymin == headAnn.ymax))
                annotations.heads.push_back(headAnn);
        }

        i++;
        widthUsed = ann.xmax;
        int interWidth = ann.xmax - ann.xmin;
        interWidth *= 0.15f;
        if(rand() % 2){
            widthUsed += rand() % interWidth;
        }
        else{
            widthUsed -= rand() % interWidth;
        }
    }while(true);
    end:
    saveImage(dest, annotations);
    createAnnotationFile(annotations);
}

std::string getMaskPath(std::string filename){
    size_t index = filename.find_last_of("/");
    std::string tmp = filename.substr(0,index);
    size_t ind = tmp.find_last_of("/");
    tmp = filename.substr(0,ind);
    
    std::string tmp2 = filename.substr(index,filename.size());
    size_t indd = tmp2.find_last_of(".");
    std::string tmp3 = tmp2.substr(0,indd);
    tmp += "/mask" + tmp3 + "mask" + ".png";
    //std::cout << filename << std::endl << tmp << std::endl << tmp2 << std::endl;
    return tmp;
}

int main(){
    srand(time(NULL));


    
    std::vector<std::string> foreFolders;
    std::vector<std::string> backFolders;
    
    core = HCore::init(16,1);

    foreFolders.push_back("/media/xoxo/2f03a838-cd91-4e2b-bfbb-968314f77511/chicken/crop/img/");

    
    backFolders.push_back("/media/xoxo/2f03a838-cd91-4e2b-bfbb-968314f77511/backgrounds/dust/img/");
    backFolders.push_back("/media/xoxo/2f03a838-cd91-4e2b-bfbb-968314f77511/backgrounds/cache/img/");
    backFolders.push_back("/media/xoxo/2f03a838-cd91-4e2b-bfbb-968314f77511/backgrounds/mirage/img/");


    std::string filepath;
    std::string maskpath;

    static int id = 0;

    for(auto &src : foreFolders){
        DIR *dir;
        dirent *ent;
        dir = opendir(src.c_str());
        int xmin,xmax,ymin,ymax;
        if(dir != NULL){
            while(ent = readdir(dir)){
                if(ent->d_name[0] != '.'){
                    //printf("File %s\n",ent->d_name);
                    filepath.clear();
                    filepath = std::string(src) + std::string(ent->d_name);
                    maskpath = getMaskPath(filepath);
                    masks.push_back(maskpath);
                    foreGrounds.push_back(filepath);
                    foreGroundClassName.push_back(std::to_string(id));
                }
            }
        }
        id++;
    }

    for(auto &src : backFolders){
        //std::cout << src << std::endl;
        DIR *dir;
        dirent *ent;
        dir = opendir(src.c_str());
        int xmin,xmax,ymin,ymax;
        if(dir != NULL){
            while(ent = readdir(dir)){
                if(ent->d_name[0] != '.'){
                    //printf("File %s\n",ent->d_name);
                    filepath.clear();
                    filepath = std::string(src) + std::string(ent->d_name);
                    backGrounds.push_back(filepath);
                }
            }
        }
    }

    uint16_t key = core->generateThreadKey();
    core->tPool->resetTaskWaiting(key);

    for(int i=0;i<AMOUNT;i++){
        core->tPool->enqueueBlocking(key,createImage,i);
        //createImage(i);
        
    }
    
    core->tPool->waitForTasks(key,0);
    

    return 0;
}