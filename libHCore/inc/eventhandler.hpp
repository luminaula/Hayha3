#ifndef EVENTHANDLER_HPP
#define EVENTHANDLER_HPP

#include "threadable.hpp"
#include "time.hpp"
#include <queue>
#include <functional>
#include <unordered_map>
#include <condition_variable>
#include <mutex>

namespace HCore{

    struct HEvent{
        uint64_t m_epoch;
        uint64_t m_id;

        timeStamp m_time;

        std::shared_ptr<std::packaged_task<void()> > m_task;

        bool operator()(const HEvent &left, const HEvent &right){
            return left.m_time > right.m_time;
        }

    };



    class EventHandler{
    private:
        std::priority_queue<HEvent,std::vector<HEvent>,HEvent> m_eventQueue;
        std::unordered_map<uint64_t,std::atomic<bool>> m_recurringEventFlags;
        std::atomic<uint64_t> m_recurringEventCounter;
        std::mutex m_mutex;
        std::mutex m_conditionMutex;
        std::condition_variable m_cv;
        bool m_running;
        HCore *m_core;
        std::thread *m_thread;
    public:
        EventHandler(HCore *core);
        ~EventHandler();

        template<class F, class... Args>
        void addEvent(timeStamp eTime, F&& f, Args&&... args){

            HEvent event;
            event.m_time = eTime;
            event.m_id = 0;
            event.m_epoch = 0;
            event.m_task = std::make_shared< std::packaged_task<void()> >(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_eventQueue.emplace(event);

            }
            m_cv.notify_one();
        }

        template<class F, class... Args>
        uint64_t addRecurringEvent(timeStamp eTime,uint64_t epoch, F&& f, Args&&... args){

            HEvent event;
            event.m_time = eTime;
            event.m_task = std::make_shared< std::packaged_task<void()> >(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
            event.m_epoch = epoch;
            event.m_id = m_recurringEventCounter++;
            m_recurringEventFlags[event.m_id] = true;
            {
                std::lock_guard<std::mutex> lock(m_mutex);   
                m_eventQueue.emplace(event);
            }
            m_cv.notify_one();
            return event.m_id;
        }

        void stopRecurringEvent(uint64_t eventId);

        void work();

        void notify(){
            m_cv.notify_one();
        }

        void startThread(){
            m_running = true;
            m_thread = new std::thread(&EventHandler::work,this);
        }

    };

}

#endif