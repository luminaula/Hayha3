#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <vector>
#include <mutex>
#include <list>
#include "bbox.hpp"
#include "tracker.hpp"
#include <stdio.h>

int offsetX,offsetY,frameWidth,frameHeight,numFrames,currentFrameNum;
std::vector<cv::Ptr<cv::Tracker>> trackers;
std::vector<cv::Rect2d> trackedBoxes;
cv::Ptr<cv::MultiTracker> multiTracker;
cv::Mat fb;
std::list<std::vector<bbox_t>> lastBoxes;
std::mutex trackmutex;
bool busy = false;

void initTracker(int x, int y, int width,int height,int numFrames_){
    setDimensions(x,y,width,height);
    numFrames = 2;
    
}

void setDimensions(int x, int y, int width,int height){
    
    offsetX = x;
    offsetY = y;
    frameWidth = width;
    frameHeight = height;
    //if(width != fb.cols || height != fb.rows)
      //  
}

void bufferToMat(unsigned char *buffer, cv::Mat &mat){
    for(int i=0; i<frameHeight;i++){
        unsigned char *fbptr = mat.ptr<unsigned char>(i);
        memcpy(fbptr,&buffer[i*frameWidth*4],frameWidth*4);
    }
}

void resetTracker(unsigned char *buffer, std::vector<bbox_t> boxes){
    std::lock_guard<std::mutex> lock(trackmutex);
    trackedBoxes.clear();
    if(boxes.empty())
        return;
    
    
    busy = true;
    fb = cv::Mat(frameHeight,frameWidth,CV_8UC4);
    bufferToMat(buffer,fb);
    multiTracker = cv::MultiTracker::create();
    
    for(int i=0; i<boxes.size();i++){
        //trackers.push_back(cv::TrackerMOSSE::create());
        cv::Rect2d roi;
        roi.x = boxes[i].x;
        roi.y = boxes[i].y;
        roi.width = boxes[i].w;
        roi.height = boxes[i].h;
        trackedBoxes.push_back(roi);
        multiTracker->add(cv::TrackerCSRT::create(),fb,roi);
    }
    
    //multiTracker->add(trackers,fb,trackedBoxes);
    busy = false;
    currentFrameNum = 0;
    
}

std::vector<bbox_t> trackFrame(unsigned char *buffer){
    cv::Mat tb = cv::Mat(frameHeight,frameWidth,CV_8UC4);
    bufferToMat(buffer,tb);
    std::lock_guard<std::mutex> lock(trackmutex);
    std::vector<bbox_t> boxes;
    if(currentFrameNum >= numFrames || trackedBoxes.empty() || busy)
        return boxes;
    
    
    
    multiTracker->update(tb,trackedBoxes);
    for(auto &roi : trackedBoxes){
        bbox_t box;
        //cv::Rect2d roi = multiTracker->getObjects()[i];
        box.x = roi.x;
        box.y = roi.y;
        box.w = roi.width;
        box.h = roi.height;
        boxes.push_back(box);
    }
    currentFrameNum++;
    
    return boxes;
}



