
#include "workers.hpp"



namespace Workers{

    bool running = false;

    HDetector *detect;
    Capturer *capture;
    StatServer *statServer;
    ApiWorker *apiworker;
    MouseWorker *mouseWorker;
    KeyboardWorker *keyboardWorker;
    HCore::HCore *m_core;

    void init(HCore::HCore *core){
        m_core = core;
        capture = new Capturer(core);
        detect = new HDetector(core);
        statServer = new StatServer(core);
        mouseWorker = new MouseWorker(core);
        keyboardWorker = new KeyboardWorker(core);
        apiworker = new ApiWorker(core);
        apiworker->reload();
        
        
    }

    void setLimit(bool value){
        capture->setLimit(value);
        detect->setLimit(value);
    }

    void start(){
        if(!running){
            if(Settings::capture.async){
                capture->startEventThread();
            }

            
            apiworker->startEventThread();
            mouseWorker->startEventThread();
            keyboardWorker->startEventThread();
            detect->startEventThread();
            running = true;
        }
        else{
            capture->stopThread();
            detect->stopThread();
            apiworker->stopThread();
            mouseWorker->stopThread();
            keyboardWorker->stopThread();
            while(detect->isRunning() || capture->isRunning()){
                std::this_thread::sleep_for(milliseconds(1));
            }
            running = false;
        }
        
    }

}