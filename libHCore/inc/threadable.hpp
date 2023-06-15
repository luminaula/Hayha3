#ifndef THREADABLE_HPP
#define THREADABLE_HPP

#include <thread>
#include <mutex>
#include <atomic>
#include <stdint.h>
#include <future>
#include "time.hpp"
using namespace std::chrono;


//Base of workers
namespace HCore{
    struct HCore;
}

class Threadable{
protected:
    float m_workPerSecond;
    uint32_t m_workTimeUsec;

    bool m_limit;

    std::atomic<bool> m_running;
    std::atomic<bool> m_busy;
    timeStamp startTime;
    timeStamp endTime;
    int m_accumulator;
 
    std::condition_variable m_cv;
    std::mutex m_mutex;
    std::mutex m_conditionMutex;
    std::string m_threadName;

    HCore::HCore *m_core;

public:
    Threadable(float workPerSecond, std::string name, bool warnings, HCore::HCore *core);
    ~Threadable();
    void setRate(float workPerSecond);
    void setLimit(bool limit);
    float getRate();

    bool isBusy();

    void conditionLock();
    void conditionLock(uint32_t timeUsec);
    void notify();

    bool isRunning();
    void startLoop();
    void endLoop();

    bool startEventLoop(timeStamp sTime);
    void endEventLoop();

    virtual void work(){}
    void singleLoop();
    void workerThread();
    void workerEventThread(timeStamp sTime);

    void startThread();
    void startEventThread();

    void stopThread();

};

#endif