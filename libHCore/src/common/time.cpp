
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

int64_t unixTime(timeStamp t0){
    return duration_cast<seconds>(t0.time_since_epoch()).count();
}


/*

#include "time.hpp"


timeStamp programStart;

nanoseconds timespecToDuration(timespec ts){
    auto duration = seconds{ts.tv_sec} + nanoseconds{ts.tv_nsec};
     return duration_cast<nanoseconds>(duration);
}

time_point<system_clock, nanoseconds>timespecToTimePoint(timespec ts){
    return time_point<system_clock, nanoseconds>{duration_cast<system_clock::duration>(timespecToDuration(ts))};
}


timeStamp getCurrentTimeMicro(){
    timeStamp ts;
    timespec_get(&ts.time, TIME_UTC);
    return ts;
}
timeStamp getTimeInFuture(uint64_t usec){
    timeStamp ts = getCurrentTimeMicro();
    return ts + usec;
}
int64_t timeSince(timeStamp t0){
    timeStamp ts = getCurrentTimeMicro() - t0;

    int64_t since = ts.time.tv_sec * 1000000 + ts.time.tv_nsec / 1000;

    return since;   
    
}

int64_t timeTo(timeStamp t0){
    return t0.micros();
}   


int64_t getTimeDifference(timeStamp t0, timeStamp t1){

}
int64_t timeSinceStart(timeStamp t0){

}
int64_t unixTime(timeStamp t0);

*/