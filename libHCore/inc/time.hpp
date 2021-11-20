
#ifndef TIME_HPP
#define TIME_HPP

#include <chrono>

using namespace std::chrono;

using timeStamp = time_point<steady_clock,microseconds>;
using timeStampSeconds = time_point<steady_clock,seconds>;

extern timeStamp programStart;


timeStamp getCurrentTimeMicro();
timeStamp getTimeInFuture(uint64_t usec);
int64_t timeSince(timeStamp t0);
int64_t timeTo(timeStamp t0);
int64_t getTimeDifference(timeStamp t0, timeStamp t1);
int64_t timeSinceStart(timeStamp t0);
int64_t unixTime(timeStamp t0);

#endif

/*
#ifndef TIME_HPP
#define TIME_HPP

#include <chrono>
#include <time.h>

using namespace std::chrono;

static inline int fastfloor(float fp) {
    int i = static_cast<int>(fp);
    return (fp < i) ? (i - 1) : (i);
}

nanoseconds timespecToDuration(timespec ts);

time_point<system_clock, nanoseconds>timespecToTimePoint(timespec ts);

struct timeStamp{
    timespec time;

    timeStamp operator-(timeStamp &t1){
        if(t1.time.tv_nsec > time.tv_nsec){
            time.tv_sec--;
            time.tv_nsec = 999999999 + time.tv_nsec - t1.time.tv_nsec;
        }
        else{
            time.tv_nsec -= t1.time.tv_nsec;
        }
        time.tv_sec -= t1.time.tv_sec;
        return *this;
    }
    timeStamp operator+(timeStamp &t1){
        if(t1.time.tv_nsec + time.tv_nsec > 999999999){
            
        }
        else{
            time.tv_sec += t1.time.tv_sec;
            time.tv_nsec += t1.time.tv_nsec;
        }
        return *this;
    }

    timeStamp operator-(uint64_t usec){
        long nsec = usec * 1000;
        int secx = fastfloor(usec / 1000000);
        
        time.tv_sec -= secx;
        nsec -= secx * 1000000000;

        if(nsec > time.tv_nsec){
            time.tv_sec--;
            time.tv_nsec = 999999999 + time.tv_nsec - nsec;
        }
        else{
            time.tv_nsec -= nsec;
        }
        return *this;
    }

    timeStamp operator+(uint64_t usec){
        long nsec = usec * 1000;
        int secx = fastfloor(usec / 1000000);
        
        time.tv_sec -= secx;

        nsec -= secx * 1000000000;

        if(nsec + time.tv_nsec > 999999999){
            time.tv_sec++;
            time.tv_nsec = (nsec + time.tv_nsec) - 999999999;
        }
        else{
            time.tv_nsec += nsec;
        }
        return *this;
    }
    

    friend bool operator> (const timeStamp t1, const timeStamp t2){
        if(t1.time.tv_sec > t2.time.tv_sec)
            return true;
        else
            return false;
        if(t1.time.tv_nsec > t2.time.tv_nsec)
            return true;
        else
            return false;
    }

    int64_t micros(){
        return time.tv_sec * 1000000 + time.tv_nsec / 1000;
    }

};

extern timeStamp programStart;





timeStamp getCurrentTimeMicro();
timeStamp getTimeInFuture(uint64_t usec);
int64_t timeSince(timeStamp t0);
int64_t timeTo(timeStamp t0);
int64_t getTimeDifference(timeStamp t0, timeStamp t1);
int64_t timeSinceStart(timeStamp t0);
int64_t unixTime(timeStamp t0);

#endif
*/