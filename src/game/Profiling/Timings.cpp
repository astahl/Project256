//
//  timings.cpp
//  Project256
//
//  Created by Andreas Stahl on 11.06.22.
//
#define CXX
#define PROFILING 1

#ifdef PROFILING

#include "Timings.h"
#include <iostream>
#include <array>
#include <string>

#endif

extern "C" {

#ifdef PROFILING

static const std::array<std::string, TimingIntervalCount> sIntervalNames {
    "TickToTick",
    "FrameToFrame",
    "TickSetup",
    "TickDo",
    "TickPost",
    "BufferCopy",
    "DrawWaitAndSetup",
    "DrawEncoding",
    "DrawPresent"
};

void profiling_time_initialise(TimingData* data) {
    data->zero = data->getPlatformTimeMicroseconds();
    std::cout << "Started at: " << data->zero << "\n";
    auto printDelay = data->getPlatformTimeMicroseconds() - data->zero;
    std::cout << "Printing that took: " << printDelay << " microseconds\n";
}

void profiling_time_set(TimingData* data, TimingTimer timer)
{
    data->timers[timer] = data->getPlatformTimeMicroseconds();
}

void profiling_time_interval(TimingData* data, TimingTimer timer, TimingInterval interval)
{
    auto now = data->getPlatformTimeMicroseconds();
    auto elapsed = now - data->timers[timer];
    data->intervals[interval] += elapsed;
    data->timers[timer] = now;
    data->intervalCount[interval]++;
}

void profiling_time_print(TimingData* data)
{
    for (int interval = 0; interval < TimingIntervalCount; ++interval)
    {
        auto count = data->intervalCount[interval];
        std::cout << sIntervalNames[interval] << " ["<< count <<"]: " << (count != 0? data->intervals[interval] / count : 0LL) << " ";
    }
    std::cout << std::endl;
}

void profiling_time_clear(struct TimingData* data)
{
    std::memset(data->intervals, 0, TimingIntervalCount * sizeof(int64_t));
    std::memset(data->intervalCount, 0, TimingIntervalCount * sizeof(int64_t));
}

#else

void profiling_time_initialise(TimingData*) {}

void profiling_time_set(TimingData*, TimingTimer) {}

void profiling_time_interval(TimingData*, TimingTimer, TimingInterval) {}

#endif
}
