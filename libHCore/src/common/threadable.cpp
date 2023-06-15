#include "hcore.hpp"


Threadable::Threadable(float workPerSecond, std::string name, bool warnings, HCore::HCore *core):
        m_core(core),
        m_workPerSecond(workPerSecond),
        m_workTimeUsec(1000000/m_workPerSecond),
        m_running(false),
        m_busy(false),
        m_limit(true),
        m_accumulator(0),
        m_threadName(name){
    startTime = getCurrentTimeMicro();
    endTime = getCurrentTimeMicro();

}
Threadable::~Threadable(){
    while(m_running){
        notify();
        stopThread();
    }
}

void Threadable::setRate(float workPerSecond){
    m_workPerSecond = workPerSecond;
    m_workTimeUsec = 1000000/m_workPerSecond;
}

float Threadable::getRate(){
    return m_workPerSecond;
}

bool Threadable::isBusy(){
    return m_busy.load();
}

bool Threadable::isRunning(){
    return m_running.load();
}

void Threadable::setLimit(bool limit){
    m_limit = limit;
}

void Threadable::conditionLock(){
    std::unique_lock<std::mutex> cLock(m_conditionMutex);
    m_cv.wait(cLock);
}

void Threadable::conditionLock(uint32_t timeUsec){
    std::unique_lock<std::mutex> cLock(m_conditionMutex);
    m_cv.wait_for(cLock,microseconds(timeUsec));
}

void Threadable::notify(){
    m_cv.notify_one();
}

void Threadable::startLoop(){
    if(m_busy)
        return;
    startTime = getCurrentTimeMicro();
    m_busy = true;
}

void Threadable::endLoop(){
    m_busy = false;
    endTime = getCurrentTimeMicro();
    uint32_t timeMikro = getTimeDifference(endTime, startTime);
    if(timeMikro < m_workTimeUsec){
        int sleeptime = m_workTimeUsec-timeMikro-m_accumulator;
        if(m_limit)
            std::this_thread::sleep_for(microseconds(sleeptime));
        m_accumulator = 0;
    }
    else{
        m_accumulator = timeMikro - m_workTimeUsec;
    }
}

bool Threadable::startEventLoop(timeStamp sTime){
    if(m_busy)
        return false;
    if(timeSince(sTime) > 2000)
        sTime = getCurrentTimeMicro();
    startTime = sTime;

    m_busy = true;
    return true;
}
void Threadable::endEventLoop(){
    m_busy = false;
    endTime = getCurrentTimeMicro();
    if(m_running){
        timeStamp nextTime = startTime+microseconds(m_workTimeUsec);
        if(timeSince(nextTime) > 0 || !m_limit)
            nextTime = endTime;
        m_core->eventHandler->addEvent(nextTime,&Threadable::workerEventThread,this,nextTime);
    }
}

void Threadable::singleLoop(){
    if(m_busy)
        return;
    //LOG(//LOG_DEBUG,"%s::singleLoop()",m_threadName.c_str());
    m_busy = true;
    work();
    m_busy = false;
}

void Threadable::startThread(){
    if(!m_running){
        m_core->log(LOG_INFO,"Start worker %s",m_threadName.c_str());
        m_running = true;
        m_core->tPool->enqueuev(&Threadable::workerThread,this);
        //LOG(//LOG_DEBUG,"%s::startThread()",m_threadName.c_str());
    }
}

void Threadable::workerThread(){
    while(m_running){
        startLoop();
        work();
        endLoop();
    }
}

void Threadable::workerEventThread(timeStamp sTime){
    if(!startEventLoop(sTime))
        return;
    work();
    
    endEventLoop();
}


void Threadable::startEventThread(){
    if(!m_running){
        m_running = true;
        m_core->tPool->enqueuev(&Threadable::workerEventThread,this,getCurrentTimeMicro());
        m_core->log(LOG_INFO,"Start worker %s",m_threadName.c_str());
    }
    else{
    }
}

void Threadable::stopThread(){
    m_core->log(LOG_INFO,"Stop worker %s",m_threadName.c_str());
    m_running = false;
    notify();
}

