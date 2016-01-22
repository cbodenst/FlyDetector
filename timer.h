#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <ctime>
#include <iomanip>
#include <string>
#include <sstream>

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::duration<int>         Duration;
typedef std::chrono::seconds               Seconds;
typedef std::chrono::time_point<Clock>     Timepoint;

template <typename T>
int convertToInt(const T& time)
{
    return std::chrono::duration_cast<Seconds>(time).count();
}

template <typename T>
std::string timeToString(T&& time)
{
    auto a_time_t = Clock::to_time_t(time);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&a_time_t), "%Y-%m-%d %H:%M:%S");

    return ss.str();
}

#endif // TIMER_H
