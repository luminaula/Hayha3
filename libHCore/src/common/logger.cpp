#include "hcore.hpp"
#include <stdlib.h> //malloc and realloc
#include <string.h>
#include <stdio.h>


namespace HCore{


    LogHandler::LogHandler(HCore *core):
            Threadable(500,"LogHandler",true, core){
        
        uint32_t logSize = 0x1000000;
        m_log.data = (char*)malloc(logSize*sizeof(char));//new char[0x4000];
        m_log.size = 0;
        m_log.total = 0x1000000;
        m_filename = "HayhaLog.txt";
        m_logFile.open(m_filename, std::ofstream::out | std::ofstream::trunc);
        m_print = true;
        
    }

    LogHandler::~LogHandler(){
        flush();
        m_logFile.close();
    }

    void LogHandler::log(uint32_t level, const char *format, ...){
        if(level < m_level)
            return;
        va_list logArgs;
        va_start(logArgs,format);
        std::lock_guard<std::mutex> lock(m_log.mutex);
        char *entryLocation = &m_log.data[m_log.size];
        int offset = logTime(level);
        int entryLength = vsprintf(entryLocation+offset,format,logArgs);
        entryLocation[entryLength + offset] = '\n';
        m_log.size += entryLength + offset + 1;
        /*
        int entryLength = strlen(entry);
        char *entryLocation = &m_log.data[m_log.size];
        int offset = logTime();
        memcpy(entryLocation + offset,entry,entryLength);
        entryLocation[entryLength + offset] = '\n';
        m_log.size += entryLength + offset + 1;
        //fprintf(stderr,"%s",m_log.entries.back());
        */
    }

    void LogHandler::log(uint32_t level, std::string &entry){
        
        std::lock_guard<std::mutex> lock(m_log.mutex);
        char *entryLocation = &m_log.data[m_log.size];
        int offset = logTime(level);
        size_t copyLen = entry.copy(entryLocation+offset,entry.size(),0);
        entryLocation[copyLen+offset] = '\n';
        m_log.size += copyLen + offset + 1;
        
    }

    void LogHandler::print(){
        flush();
    }

    int LogHandler::logTime(uint32_t level){
        char *entryLocation = &m_log.data[m_log.size];
        int64_t currentTime = timeSince(programStart);
        uint32_t hours,minutes,seconds,millis,micros;
        seconds = currentTime/1000000;
        hours = seconds/3600;
        minutes = seconds/60 % 60;
        seconds %= 60;
        millis = currentTime/1000 % 1000;
        micros = currentTime%1000;
        switch(level){
            case LOG_DEBUG:
                return sprintf(entryLocation,"[DEBUG][%.2u:%.2u:%.2u.%.3u,%.3u] ",hours,minutes,seconds,millis,micros);
                break;
            case LOG_INFO:
                return sprintf(entryLocation,"[INFO][%.2u:%.2u:%.2u.%.3u,%.3u] ",hours,minutes,seconds,millis,micros);
                break;
            case LOG_WARNING:
                return sprintf(entryLocation,"[WARNING][%.2u:%.2u:%.2u.%.3u,%.3u] ",hours,minutes,seconds,millis,micros);
                break;
            case LOG_ERROR:
                return sprintf(entryLocation,"[ERROR][%.2u:%.2u:%.2u.%.3u,%.3u] ",hours,minutes,seconds,millis,micros);
                break;
            default:
                return sprintf(entryLocation,"[UNK][%.2u:%.2u:%.2u.%.3u,%.3u] ",hours,minutes,seconds,millis,micros);
                break;
        }
    }

    void LogHandler::flush(){
        std::lock_guard<std::mutex> lock(m_log.mutex); 
        m_logFile.write(m_log.data,m_log.size);
        m_logFile.flush();
        if(m_print)
            std::cout.write(m_log.data,m_log.size);
        m_log.size = 0;

    }


    void LogHandler::work(){
        if(m_log.size > 1){
            flush();
        }
    }

}