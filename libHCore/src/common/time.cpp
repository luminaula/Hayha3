#include "time.hpp"

timeStamp programStart;


timeStamp getCurrentTimeMicro(){
    return time_point_cast<microseconds>(steady_clock::now());
}

timeStamp getTimeInFuture(uint64_t usec){
    return time_point_cast<microseconds>(steady_clock::now()) + std::chrono::microseconds(usec);
}

int64_t getTimeDifference(timeStamp t0, timeStamp t1){
    return duration_cast<microseconds>(t0 - t1).count();
}

int64_t timeSince(timeStamp t0){
    return duration_cast<microseconds>(getCurrentTimeMicro() - t0).count();
}
int64_t timeTo(timeStamp t0){
    return duration_cast<microseconds>(t0-getCurrentTimeMicro()).count();
}

int64_t timeSinceStart(timeStamp t0){
    return duration_cast<microseconds>(t0 - programStart).count();
}