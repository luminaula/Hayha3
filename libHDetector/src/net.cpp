#include "net.hpp"
#include <string>
#include <stdio.h>
#include <iostream>



HNetCV::HNetCV(char *cfg, char  *weight, int gpu_id){
    m_net = cv::dnn::readNetFromDarknet(weight,cfg);
}
HNetCV::HNetCV(){

}
HNetCV::~HNetCV(){

}




std::vector<bbox_t> HNetCV::detect(image_t img, float thresh){
    cv::Mat imgg(img.h,img.w,CV_8UC3);
    cv::Mat blob;
    cv::dnn::blobFromImage(imgg,blob);
    m_net.setInput(blob);
    std::vector<cv::String> outNames = m_net.getUnconnectedOutLayersNames();
    std::vector<cv::Mat> outs;
    m_net.forward(outs, outNames);

    std::vector<bbox_t> bbox;

    static std::vector<int> outLayers = m_net.getUnconnectedOutLayers();
    static std::string outLayerType = m_net.getLayer(outLayers[0])->type;

    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    float confThreshold = thresh;
    float nmsThreshold = .4;

    

    if (m_net.getLayer(0)->outputNameToIndex("im_info") != -1)  // Faster-RCNN or R-FCN
    {
        // Network produces output blob with a shape 1x1xNx7 where N is a number of
        // detections and an every detection is a vector of values
        // [batchId, classId, confidence, left, top, right, bottom]
        float* data = (float*)outs[0].data;
        for (size_t i = 0; i < outs[0].total(); i += 7)
        {
            float confidence = data[i + 2];
            if (confidence > confThreshold)
            {
                int left = (int)data[i + 3];
                int top = (int)data[i + 4];
                int right = (int)data[i + 5];
                int bottom = (int)data[i + 6];
                int width = right - left + 1;
                int height = bottom - top + 1;
                classIds.push_back((int)(data[i + 1]) - 1);  // Skip 0th background class id.
                boxes.push_back(cv::Rect(left, top, width, height));
                confidences.push_back(confidence);
            }
        }
    }
    else if (outLayerType == "DetectionOutput")
    {
        // Network produces output blob with a shape 1x1xNx7 where N is a number of
        // detections and an every detection is a vector of values
        // [batchId, classId, confidence, left, top, right, bottom]
        float* data = (float*)outs[0].data;
        for (size_t i = 0; i < outs[0].total(); i += 7)
        {
            float confidence = data[i + 2];
            if (confidence > confThreshold)
            {
                int left = (int)(data[i + 3] * img.w);
                int top = (int)(data[i + 4] * img.h);
                int right = (int)(data[i + 5] * img.w);
                int bottom = (int)(data[i + 6] * img.h);
                int width = right - left + 1;
                int height = bottom - top + 1;
                classIds.push_back((int)(data[i + 1]) - 1);  // Skip 0th background class id.
                boxes.push_back(cv::Rect(left, top, width, height));
                confidences.push_back(confidence);
            }
        }
    }
    else if (outLayerType == "Region")
    {
        for (size_t i = 0; i < outs.size(); ++i)
        {
            // Network produces output blob with a shape NxC where N is a number of
            // detected objects and C is a number of classes + 4 where the first 4
            // numbers are [center_x, center_y, width, height]
            float* data = (float*)outs[i].data;
            for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
            {
                
                cv::Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
                cv::Point classIdPoint;
                std::cout << scores << std::endl;
                double confidence;
                cv::minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
                printf("%f ",confidence);
                for(int k =0;k<scores.rows;k++){
                    //printf("%f\n",scores.at(k));
                }
                    
                int centerX = (int)(data[0] * img.w);
                int centerY = (int)(data[1] * img.h);
                int width = (int)(data[2] * img.w);
                int height = (int)(data[3] * img.h);
                int left = centerX - width / 2;
                int top = centerY - height / 2;

                classIds.push_back(classIdPoint.x);
                confidences.push_back((float)confidence);
                boxes.push_back(cv::Rect(left, top, width, height));
                bbox_t boxx;
                boxx.x = left + img.detectionX;
                boxx.y = top + img.detectionY;
                boxx.w = width;
                boxx.h = height;
                bbox.push_back(boxx);
            
            }
        }
    }

    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
    for(auto &boxie : bbox){
        //printf("Asd\n");
    }

    return bbox;

}
void HNetCV::train(int argc,char **argv){

}

INeuralNet* createDetector(char *cfg, char  *weight, int gpu_id){
    return new HNetCV(cfg,weight,0);
}
