#ifndef HCORE_HPP
#define HCORE_HPP

#include "eventhandler.hpp"
#include "hthread.hpp"
#include "threadable.hpp"
#include "logger.hpp"
#include "time.h"

class EventHandler;
class LogHandler;
class ThreadPool;




namespace HCore{

    extern HCore *core;

    struct HCore{
        ThreadPool *tPool;
        LogHandler *logger;
        EventHandler *eventHandler;

        HCore(uint32_t threadCount, uint32_t loglevel);

        template<class ...T>
        void log(T&& ... args){
            logger->log(std::forward<T>(args)...);
        }

        uint16_t generateThreadKey();

        

    };

    template<class ...T>
    void log(T&& ... args){
        core->log(std::forward<T>(args)...);
    }


    HCore *mainCore();

    

    HCore* init(uint32_t threadCount, uint32_t loglevel);


    

};

void threadedSleep(uint32_t sleepTime);


#endif