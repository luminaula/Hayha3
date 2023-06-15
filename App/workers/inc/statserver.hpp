#ifndef STATSERVER_HPP
#define STATSERVER_HPP
#include "threadable.hpp"
#include <map>
#include <string>
#include <list>

struct Stat{
    std::list<float> previous;
    uint32_t counter;
    float value;
};

class StatServer : public Threadable{
private:
    /*
    std::map<std::string, 
    std::map<std::string,uint32_t> m_statMap;
    std::map<std::string,float> m_stats;
    */
    std::map<std::string,Stat> m_stats;

public:
    StatServer(HCore::HCore *core);
    ~StatServer();


    void pushStat(std::string stat);
    float getStat(std::string stat);
    void work();

};

#endif