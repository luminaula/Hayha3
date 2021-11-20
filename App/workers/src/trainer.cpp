#include "trainer.hpp"
#include "filesystem.hpp"
#include "settings.hpp"
#include "workers.hpp"

Trainer::Trainer(HCore::HCore *core) : Threadable(9999, "Trainer", false, core) {

    std::string libname = OS::libPath + OS::libPrefix + Settings::train.libfile + OS::libSuffix;
    loadLibrary(libname.c_str());

    m_detectorCreate["create"] = loadFunc<INeuralNet *>("createTrainer");

    if (m_detectorCreate["create"]){
        m_net = m_detectorCreate["create"]();

        std::string name = OS::datasetPath + "images/";
        readFolder(name);
        m_net->setTrainData(m_trainData);
    }
}

void Trainer::addImage(std::string &filename) { m_trainData.push_back(filename); }

void Trainer::clearData() { m_trainData.clear(); }

void Trainer::readFolder(std::string &foldername) {
    std::vector<std::string> ext;
    ext.push_back(".jpg");
    ext.push_back(".png");
    m_trainData = OS::readFolder(foldername, ext);
}

void Trainer::work() {
    TrainInfo info;
    std::string conf = OS::datasetPath + "net.cfg";
    std::string weight = OS::datasetPath + "net.weight";
    info.config = new char[conf.length() + 1];
    info.weight = new char[weight.length() + 1];
    sprintf(info.config, "%s", conf.c_str());
    sprintf(info.weight, "%s", weight.c_str());
    info.gpus = Settings::train.gpus;
    int dim = (rand() % 9) + 10;
    info.width = dim * 32;
    info.height = info.width;
    printf("%d\n",info.height);
    info.iterations = Settings::train.iters;

    m_net->train2(info);

    delete info.weight;
    delete info.config;
}