#ifndef HDETECTOR_HPP
#define HDETECTOR_HPP

#include <vector>
#include "hcore.hpp"
#include "INeuralNet.hpp"
#include "module.hpp"




class HDetector : public Threadable, public Module{
private:
    INeuralNet *yoloDetector;
    std::vector<bbox_t> m_detections;

    void detectRemote(image_t img);
    void detectLocal(image_t img);



public:
    HDetector(HCore::HCore *core);

    MFunc<INeuralNet*> m_detectorCreate;


    void loadDetector();


    void benchmark(int height,int width,int times);
    void benchmark();
    void work();
};



#endif