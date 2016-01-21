#ifndef TIMER_H
#define TIMER_H

#include <chrono>

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::duration<int>         Duration;
typedef std::chrono::seconds               Seconds;
typedef std::chrono::time_point<Clock>     Timepoint;

template <typename T>
int convertToInt(const T& time)
{
    return std::chrono::duration_cast<Seconds>(time).count();
}

#endif // TIMER_H
