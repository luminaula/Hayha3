
#pragma once

#include <vector>
#include <queue>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <memory>
#include <stdexcept>
#include <future>
#include <unordered_map>

#include <iostream>
#include <stdio.h>
#include "hcore.hpp"
#include "blockingconcurrentqueue.h"


namespace HCore{

    enum ThreadState : uint32_t{
        IDLE = 0,
        WORK = 1
    };

    struct ThreadStateCount{
        uint32_t idle,work;
    };

    struct HTask{
        std::shared_ptr<std::packaged_task<void()> > m_task;
        uint16_t taskKey;
    };

    struct HTaskCookie{
        std::condition_variable m_taskCondition;
        std::mutex m_taskMutex;
        std::atomic<uint32_t> m_taskCount;
    };

    class ThreadPool;

    class HThread : public std::thread{
    private:
        ThreadState state;
        ThreadPool *tPool;
        uint16_t currentTaskKey;
    public:
        void run();
        ThreadState getState();
        static void loader(ThreadPool *pool, uint32_t index);
        HThread(std::function<void()> fun, ThreadPool *pool);
    };


    class ThreadPool{

    private:

        std::vector<HThread*> workers;
        moodycamel::BlockingConcurrentQueue<HTask> taskQueueC;
        std::unordered_map<uint16_t,std::atomic<uint32_t>> taskMap;
        std::condition_variable *m_taskConditions;
        std::mutex *m_taskMutexes;
        HTaskCookie *m_taskCookies;
        bool running;
        HCore *m_core;

    public:
    friend class HThread;
    
        ThreadPool(uint32_t count,HCore *core);
        ~ThreadPool();
        HThread *getThreadHandle(uint32_t index);
        size_t size(){return workers.size();}

        ThreadStateCount getThreadStates();
        int idleCount();
        void waitUntilFinished();
        
        void resetTaskWaiting(uint16_t key);
        void waitForTasks(uint16_t key, uint32_t time_);
        void finishTask(uint16_t key);


        void pushToQueue(HTask &task){
            taskQueueC.enqueue(task);
        }

            




        template<class F, class... Args>
        void enqueuev(F&& f, Args&&... args){
            HTask htask;
            htask.m_task = std::make_shared< std::packaged_task<void()> >(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
            htask.taskKey = 0;
            pushToQueue(htask);

        }

        template<class F, class... Args>
        auto enqueue(F&& f, Args&&... args)
            -> std::future<typename std::result_of<F(Args...)>::type>{
            using return_type = typename std::result_of<F(Args...)>::type;
            auto task = std::make_shared< std::packaged_task<return_type()> >(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
            std::future<return_type> res = task->get_future();
            enqueuev([task](){
                (*task)();
            });
            return res;
        }

        template<class F, class... Args>
        void enqueueBlocking(uint64_t key,F&& f, Args&&... args){

            HTask htask;
            htask.m_task = std::make_shared< std::packaged_task<void()> >(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
            htask.taskKey = key;
            m_taskCookies[key].m_taskCount++;
            pushToQueue(htask);
        }

        void enqueueEvent(std::shared_ptr<std::packaged_task<void()>> &fun);

    };

}