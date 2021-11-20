#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <stdint.h>
#include <mutex>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <stdarg.h>

#include "threadable.hpp"
#include "time.hpp"


namespace HCore{


    #define LOG_DEBUG 0
    #define LOG_INFO 1
    #define LOG_WARNING 2
    #define LOG_ERROR 3

    struct HLog{
        char *data;
        size_t size,total;
        std::mutex mutex;
        std::vector<char*> entries;
    };

    class LogHandler : public Threadable{
    private:
        HLog m_log;
        bool m_print;
        std::string m_filename;
        std::ofstream m_logFile;
        int logTime(uint32_t level);
    public:
        uint32_t m_level = 0;

        LogHandler(HCore *core);
        ~LogHandler();

        void log(uint32_t level, const char *format, ...);
        void log(uint32_t level, std::string &entry);
        void flush();
        void print();

        void work();
    };

    extern LogHandler *logger;


}

#endif