#include "hnetTRT.hpp"
#include <iostream>
#include <fstream>

float overlap(float x1, float w1, float x2, float w2)
{
    float l1 = x1 - w1/2;
    float l2 = x2 - w2/2;
    float left = l1 > l2 ? l1 : l2;
    float r1 = x1 + w1/2;
    float r2 = x2 + w2/2;
    float right = r1 < r2 ? r1 : r2;
    return right - left;
}

float box_intersection(bbox_t &a, bbox_t &b)
{
    float w = overlap(a.x, a.w, b.x, b.w);
    float h = overlap(a.y, a.h, b.y, b.h);
    if(w < 0 || h < 0) return 0;
    float area = w*h;
    return area;
}

float IOU(bbox_t &a, bbox_t &b){
    float areaIntersection, areaUnion;

    areaIntersection = box_intersection(a,b);
    areaUnion = a.area() + b.area() - areaIntersection;

    return areaIntersection / areaUnion;
}

std::vector<bbox_t> processBoxes(std::vector<bbox_t> &tboxes,image_t img){
    std::vector<bbox_t> bboxes;


    std::sort(tboxes.begin(),tboxes.end(),[](const bbox_t &left, const bbox_t &right){
        if(left.obj_id != right.obj_id){
            return left.obj_id < right.obj_id;
        }
        return left.prob > right.prob;
    });

    for(int i=0;i<tboxes.size();i++){
        for(int j=i+1; j<tboxes.size(); j++){
            if(tboxes[i].obj_id != tboxes[j].obj_id)
                break;
            if(IOU(tboxes[i],tboxes[j]) > 0.5){
                if(tboxes[j].prob <= tboxes[i].prob)
                    tboxes[j].prob = 0.0;
                else
                    tboxes[i].prob = 0.0;
            }
        }
    }

    for(int i=0;i<tboxes.size();i++){
        bbox_t box;
        if(tboxes[i].prob < img.thresh)
            continue;
        bboxes.push_back(tboxes[i]);
    }

    return bboxes;
}


std::string getType(std::string config){
    std::string line;
    std::ifstream input(config, std::ios::binary);
    while(getline(input,line)){
        if(line.find("yolo") != std::string::npos){
            return "yolov3";
        }
    }
    return "yolov2";
}


YoloNet::YoloNet(NetCreateInfo info) :
    m_info(info){
    loadNet(m_info);

}

YoloNet::~YoloNet(){

}


void YoloNet::loadNet(NetCreateInfo info){
    cudaSetDevice(info.gpu);

    if(m_net){
        m_net.reset(nullptr);
    }
    NetworkInfo yoloInfo;
    InferParams infer;

    yoloInfo.networkType = getType(info.config);
    yoloInfo.configFilePath = info.config;
    yoloInfo.wtsFilePath = info.weight;
    yoloInfo.labelsFilePath = "names.txt";
    yoloInfo.precision = "k" + std::string(info.precision);
    yoloInfo.calibrationTablePath = std::string(info.folder) + "net-calibration.table";
    yoloInfo.enginePath = "";
    yoloInfo.inputBlobName = "data";
    yoloInfo.width = info.width;
    yoloInfo.height = info.height;

    infer.printPerfInfo = false;
    infer.printPredictionInfo = false;
    infer.probThresh = 0.5;
    infer.nmsThresh = 0.5;
    infer.calibImages = "calib.txt";




    if(yoloInfo.networkType.compare("yolov3") == 0){
        m_net = std::unique_ptr<Yolo>{new YoloV3(1,yoloInfo,infer)};
    }
    else{
        m_net = std::unique_ptr<Yolo>{new YoloV2(1,yoloInfo,infer)};
    }
}

std::vector<bbox_t> YoloNet::detect(image_t img,float thresh){
    cudaSetDevice(m_info.gpu);
    std::vector<bbox_t> dets;


    if(m_net->getInputH() != img.h || m_net->getInputW() != img.w){
        m_info.width = img.w;
        m_info.height = img.h;
        loadNet(m_info);
    }



    m_net->m_ProbThresh = img.thresh;



    m_net->doInference((unsigned char*)img.data,1);
    
    auto binfo = m_net->decodeDetections(0, img.h,img.w);
    

    for(auto &b : binfo){
        bbox_t baux;
        baux.x = b.box.x1 + img.detectionX;
        baux.y = b.box.y1 + img.detectionY;
        baux.w = b.box.x2 - b.box.x1;
        baux.h = b.box.y2 - b.box.y1;
        baux.prob = b.prob;
        baux.obj_id = b.classId - 1;
        dets.push_back(baux);
    }

    return processBoxes(dets,img);
}

INeuralNet* createDetector(NetCreateInfo info){
    return new YoloNet(info);
}