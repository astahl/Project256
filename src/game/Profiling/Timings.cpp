//
//  timings.cpp
//  Project256
//
//  Created by Andreas Stahl on 11.06.22.
//
#define PROFILING 1

#ifdef PROFILING

#include "Timings.h"
#include <cstdio>
#include <array>
#include <string>
#include <shared_mutex>

#endif

extern "C" {

#ifdef PROFILING

static const std::array<std::array<char, 32>, TimingIntervalCount> sIntervalNames {
    "TickToTick",
    "FrameToFrame",
    "AudioBufToAudioBuf",
    "AudioBufFill",
    "TickSetup",
    "TickDo",
    "TickPost",
    "BufferCopy",
    "DrawBefore",
    "DrawWaitSetup",
    "DrawEncoding",
    "DrawPresent"
};

globalvar std::shared_mutex Mutex{};

void profiling_time_set(TimingData* data, TimingTimer timer)
{
    std::unique_lock lock{Mutex};
    data->timers[timer] = data->getPlatformTimeMicroseconds();
}

void profiling_time_interval(TimingData* data, TimingTimer timer, TimingInterval interval)
{
    std::unique_lock lock{Mutex};
    auto now = data->getPlatformTimeMicroseconds();
    auto elapsed = now - data->timers[timer];
    data->intervals[interval] += elapsed;
    data->timers[timer] = now;
    data->intervalCount[interval]++;
}

int profiling_time_print(TimingData* data, char* buffer, int bufferSize)
{
    std::shared_lock lock{Mutex};
    int writtenTotal = 0;
    for (int interval = 0; interval < TimingIntervalCount; ++interval)
    {
        int count = data->intervalCount[interval];
        int written = std::snprintf(buffer, bufferSize, "%-16s %-5d %lld\n", sIntervalNames[interval].data(), count, (count != 0? data->intervals[interval] / count : 0LL));
        buffer += written;
        bufferSize -= written;
        writtenTotal += written;
    }
    return writtenTotal;
}

void profiling_time_clear(struct TimingData* data)
{
    std::unique_lock lock{Mutex};
    std::memset(data->intervals, 0, TimingIntervalCount * sizeof(int64_t));
    std::memset(data->intervalCount, 0, TimingIntervalCount * sizeof(int));
}

#else

void profiling_time_initialise(TimingData*) {}

void profiling_time_set(TimingData*, TimingTimer) {}

void profiling_time_interval(TimingData*, TimingTimer, TimingInterval) {}

#endif
}
