#include "statserver.hpp"

StatServer::StatServer(HCore::HCore *core) : Threadable(5, "Stat", true, core) {}

void StatServer::pushStat(std::string stat) { m_stats[stat].counter++; }

float StatServer::getStat(std::string stat) { return m_stats[stat].value; }

void StatServer::work() {
    for (auto &entry : m_stats) {
        float stat = (float)entry.second.counter / (float)m_workTimeUsec * 1000000;

        entry.second.previous.push_back(stat);
        if (entry.second.previous.size() > getRate() * 5) {
            entry.second.previous.pop_front();
        }
        float accum = 0.0;
        for (auto &val : entry.second.previous) {
            accum += val;
        }
        accum /= entry.second.previous.size();
        entry.second.value = accum; // m_stats[entry.first] * 0.5 + stat * 0.5;
        entry.second.counter = 0;
    }
}