#include "hcore.hpp"

namespace HCore{
    
    HCore *core;

    HCore::HCore(uint32_t threadCount, uint32_t loglevel){
        logger = new LogHandler(this);
        logger->m_level = loglevel;
        tPool = new ThreadPool(threadCount,this);
        eventHandler = new EventHandler(this);

        eventHandler->startThread();
        logger->startEventThread();
    }

    HCore* init(uint32_t threadCount, uint32_t loglevel){
        programStart = getCurrentTimeMicro();
        core = new HCore(threadCount,loglevel);
        return core;
    }

    uint16_t HCore::generateThreadKey(){
        static std::mutex keymutex;
        static uint16_t count = 1;
        std::lock_guard<std::mutex>lock(keymutex);
        if(count == 0)
            count++;
        uint16_t tKey = count++;
        return tKey;
    }

    HCore *mainCore(){
        return core;
    }


};

void threadedSleep(uint32_t sleepTime){
    std::this_thread::sleep_for(microseconds(sleepTime));
}

