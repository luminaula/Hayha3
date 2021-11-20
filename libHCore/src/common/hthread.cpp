#include "hthread.hpp"
#include "hcore.hpp"
#include <limits.h>
#include <condition_variable>

namespace HCore{

    HThread::HThread(std::function<void()> fun, ThreadPool *pool)
        :   std::thread(fun),
            tPool(pool),
            currentTaskKey(0){
        
    }


    void HThread::run(){
        while(true){ 
            HTask htask;
            tPool->taskQueueC.wait_dequeue(htask);
            state = WORK;
            currentTaskKey = htask.taskKey;
            try{
                (*htask.m_task)();
            }
            catch(...){
                std::cerr << "Threadpool fucked up" << std::endl; 
            }
            if(currentTaskKey){   
                tPool->finishTask(currentTaskKey);
            }
            currentTaskKey = 0;
            state = IDLE;
        }
    }

    ThreadState HThread::getState(){
        return state;
    }

    void HThread::loader(ThreadPool *pool, uint32_t index){
        std::this_thread::sleep_for(milliseconds(500));
        pool->getThreadHandle(index)->run();
    }

    ThreadPool::ThreadPool(uint32_t count,HCore *core):
                running(true),
                m_core(core){
        workers.clear();
        workers.resize(count);
        for(uint32_t i=0;i<count;i++){
            std::function<void()> fun = std::bind(&HThread::loader,this, i);
            workers[i] = new HThread(fun, this);
        }
        m_taskCookies = new HTaskCookie[UINT16_MAX + 1];
    }

    ThreadPool::~ThreadPool(){
        running = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        for(auto &worker : workers){
            worker->join();
        }

    }

    ThreadStateCount ThreadPool::getThreadStates(){
        ThreadStateCount tsc = {0};
        for(auto &worker : workers){
            ThreadState ts = worker->getState();
            switch(ts){
                case IDLE:
                    tsc.idle++;
                    break;
                case WORK:
                    tsc.work++;
                    break;
            }
        }
        return tsc;
    }

    int ThreadPool::idleCount(){
        uint32_t idle = 0;
        for(auto &worker : workers){
            if(worker->getState() == IDLE)
                idle++;
        }
        return idle;
    }

    void ThreadPool::waitUntilFinished(){

    }

    void ThreadPool::resetTaskWaiting(uint16_t key){
        taskMap[key] = 0;
        m_taskCookies[key].m_taskCount = 0;
    }

    void ThreadPool::waitForTasks(uint16_t key, uint32_t time_){
        HTaskCookie &cookie = m_taskCookies[key];
        std::unique_lock<std::mutex> lock(cookie.m_taskMutex);
        if(cookie.m_taskCount == 0)
            return;
        if(time_ == 0){
            while(cookie.m_taskCount){
                cookie.m_taskCondition.wait_for(lock,microseconds(100),[&cookie]{return cookie.m_taskCount == 0;});
            }
        }
        else
            cookie.m_taskCondition.wait_for(lock,microseconds(time_),[&cookie]{return cookie.m_taskCount == 0;});    
    }

    void ThreadPool::finishTask(uint16_t key){
        m_taskCookies[key].m_taskCount--;
        m_taskCookies[key].m_taskCondition.notify_one();
    }


    HThread *ThreadPool::getThreadHandle(uint32_t index){
        return workers.at(index);
    }

    void ThreadPool::enqueueEvent(std::shared_ptr<std::packaged_task<void()>> &fun){
        (*fun).reset();
        HTask htask;
        htask.m_task = std::move(fun);
        htask.taskKey = 0;
        taskQueueC.enqueue(htask);

    }

}
