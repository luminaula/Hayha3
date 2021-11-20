#include "net.hpp"
#include <string>
#include <stdio.h>
#include <iostream>



HNetCV::HNetCV(char *cfg, char  *weight, int gpu_id){
    printf("asd\n");
    m_net = cv::dnn::readNetFromDarknet(cfg,weight);
    printf("asd\n");
    m_net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    m_net.setPreferableTarget(cv::dnn::DNN_TARGET_OPENCL);
}
HNetCV::HNetCV(){

}
HNetCV::~HNetCV(){

}




std::vector<bbox_t> HNetCV::detect(image_t img, float thresh){
    std::vector<bbox_t> bbox;
    std::vector<cv::String> outNames = m_net.getUnconnectedOutLayersNames();
    cv::Mat imgg(img.h,img.w,CV_8UC3,img.bdata);
    cv::Mat blob;
    std::vector<cv::Mat> outs;
    cv::dnn::blobFromImage(imgg, 1 / 255.F, cv::Size(img.h, img.w), cv::Scalar(), true, false);
    m_net.setInput(blob,"data");
    m_net.forward(outs, outNames);

    
    return bbox;

}
void HNetCV::train(int argc,char **argv){

}

INeuralNet* createDetector(char *cfg, char  *weight, int gpu_id){
    return new HNetCV(cfg,weight,0);
}
