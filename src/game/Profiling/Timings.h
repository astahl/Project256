//
//  timings.hpp
//  Project256
//
//  Created by Andreas Stahl on 11.06.22.
//

#ifndef timings_hpp
#define timings_hpp

#include "../defines.h"
#include "stdint.h"

enum TimingTimer {
    eTimerTickToTick,
    eTimerFrameToFrame,
    eTimerAudioBufferToAudioBuffer,
    eTimerFillAudioBuffer,
    eTimerTick,
    eTimerBufferCopy,
    eTimerDraw,
    TimingTimerCount
};

enum TimingInterval {
    eTimingTickToTick,
    eTimingFrameToFrame,
    eTimingAudioBufferToAudioBuffer,
    eTimingFillAudioBuffer,
    eTimingTickSetup,
    eTimingTickDo,
    eTimingTickPost,
    eTimingBufferCopy,
    eTimingDrawBefore,
    eTimingDrawWaitAndSetup,
    eTimingDrawEncoding,
    eTimingDrawPresent,
    TimingIntervalCount
};

typedef int64_t (*PlatformTimeMicrosecondsCallback)();

struct TimingData {
    PlatformTimeMicrosecondsCallback getPlatformTimeMicroseconds;
    int64_t timers[TimingTimerCount];
    int64_t intervals[TimingIntervalCount];
    int intervalCount[TimingIntervalCount];
} CF_SWIFT_NAME(ProfilingTime);


#ifdef __cplusplus
extern "C" {
#endif

GAME256_API void profiling_time_set(struct TimingData* data, enum TimingTimer timer)
    CF_SWIFT_NAME(ProfilingTime.startTimer(self:_:));
GAME256_API void profiling_time_interval(struct TimingData* data, enum TimingTimer timer, enum TimingInterval interval)
    CF_SWIFT_NAME(ProfilingTime.interval(self:timer:interval:));
GAME256_API int profiling_time_print(struct TimingData* data, char* buffer, int bufferSize)
    CF_SWIFT_NAME(ProfilingTime.printTo(self:buffer:size:));
GAME256_API void profiling_time_clear(struct TimingData* data)
    CF_SWIFT_NAME(ProfilingTime.clear(self:));


#ifdef __cplusplus
}
#endif

#endif /* timings_hpp */
