#include "bbox.hpp"
#include "INeuralNet.hpp"

#include "yolo.h"
#include "yolov2.h"
#include "yolov3.h"



class YoloNet : public INeuralNet{
private:
    std::unique_ptr<Yolo> m_net;
    NetCreateInfo m_info;
    int m_gpuId;
public:
    YoloNet(NetCreateInfo info);
    ~YoloNet();

    std::vector<bbox_t> detect(image_t img,float thresh);

    void loadNet(NetCreateInfo info);
};

extern "C" INeuralNet* createDetector(NetCreateInfo info);