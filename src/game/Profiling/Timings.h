//
//  timings.hpp
//  Project256
//
//  Created by Andreas Stahl on 11.06.22.
//

#ifndef timings_hpp
#define timings_hpp

#include "stdint.h"

enum TimingTimer {
    eTimerFrameToFrame,
    eTimerTick,
    eTimerBufferCopy,
    eTimerDraw,
    TimingTimerCount
};

enum TimingInterval {
    eTimingFrameToFrame,
    eTimingTickSetup,
    eTimingTickDo,
    eTimingTickPost,
    eTimingBufferCopy,
    eTimingDrawWaitAndSetup,
    eTimingDrawEncoding,
    eTimingDrawPresent,
    TimingIntervalCount
};

struct TimingData {
    int64_t (*getPlatformTimeMicroseconds)();
    int64_t zero;
    int64_t timers[TimingTimerCount];
    int64_t intervals[TimingIntervalCount];
    int intervalCount[TimingIntervalCount];
};


#ifdef CXX
extern "C" {
#endif

void profiling_time_initialise(struct TimingData* data);
void profiling_time_set(struct TimingData* data, enum TimingTimer timer);
void profiling_time_interval(struct TimingData* data, enum TimingTimer timer, enum TimingInterval interval);
void profiling_time_print(struct TimingData* data);
void profiling_time_clear(struct TimingData* data);


#ifdef CXX
}
#endif

#endif /* timings_hpp */
