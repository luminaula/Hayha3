#include <string>
#include "INeuralNet.hpp"
#include "hcore.hpp"
#include "module.hpp"
#include "settings.hpp"
#include "filesystem.hpp"


INeuralNet *detector;

HCore::HCore *mainCore;

class DetectorModule : public Module{
public:
    MFunc<INeuralNet*> m_detectorCreate;

    void loadcreateTrainer(){
        mainCore->log(LOG_INFO,"Loading function");
        #ifdef _WIN32
        m_detectorCreate["createTrainer"] = (moduleFunc<INeuralNet*>)GetProcAddress(m_library,"createTrainer");
        #else
        m_detectorCreate["createTrainer"] = (moduleFunc<INeuralNet*>)dlsym(m_library,"createTrainer");
        #endif
        if(m_detectorCreate["createTrainer"] == NULL){
            mainCore->log(LOG_ERROR,"Function null");
        }
        else{
            mainCore->log(LOG_INFO,"Function loaded");
        }
    }
};


#include <string.h>

int main(int argc,char **argv){
    OS::initPaths();
    Settings::loadSettings(OS::cfgPath + "Settings.json");
    mainCore = HCore::init(8, 1);

    if(strcmp(argv[1],"anchor") == 0){
        //calc_anchors(argv[2],atoi(argv[3]),416,416,0);
        return 0;
    }

    DetectorModule m_module;
    std::string libname = OS::libPath + OS::libPrefix + Settings::net.libFile + OS::libSuffix;
    m_module.loadLibrary(libname.c_str());
    m_module.loadcreateTrainer();
    detector = m_module.m_detectorCreate["createTrainer"](Settings::net.configFile,Settings::net.weightFile,0);

    detector->train(argc,argv);


    return 0;
}