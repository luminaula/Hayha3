#include "eventhandler.hpp"
#include "hcore.hpp"

namespace HCore{

    

    EventHandler::EventHandler(HCore *core):
            m_recurringEventCounter(1),
            m_core(core){

    }


    void EventHandler::stopRecurringEvent(uint64_t eventId){
        if(m_recurringEventFlags[eventId])
            m_recurringEventFlags[eventId] = false;
        else{}
            //LOG(//LOG_DEBUG,"Tried stopping non existing event");
    }


    void EventHandler::work(){
        HEvent event;
        while(m_running){
            timeStamp currentTime = getCurrentTimeMicro();
            while(!m_eventQueue.empty() && getTimeDifference(m_eventQueue.top().m_time,currentTime) < 60){
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    event = std::move(m_eventQueue.top());
                    m_eventQueue.pop();
                }
                if(event.m_id){
                    //What the fuck have I done here
                    m_core->tPool->enqueuev([this,event](){
                        (*event.m_task).reset();
                        (*event.m_task)();
                        //some trickery to modify event 
                        HEvent *eventPtr = const_cast<HEvent*> (&event);
                        eventPtr->m_time = getCurrentTimeMicro() + microseconds(event.m_epoch);
                        {
                            std::lock_guard<std::mutex> lock(m_mutex);
                            m_eventQueue.push(event);
                        }
                        notify();
                    });
                    
                }
                else{
                    m_core->tPool->enqueueEvent(event.m_task);
                }
            }

            std::unique_lock<std::mutex> lock(m_conditionMutex);
            
            if(!m_eventQueue.empty()){
                if(timeTo(m_eventQueue.top().m_time) > 30){
                    m_cv.wait_until(lock,m_eventQueue.top().m_time);
                }
            }
            else{
                m_cv.wait(lock);
            }
            

        }
        
    }

}
