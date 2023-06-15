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


#endif