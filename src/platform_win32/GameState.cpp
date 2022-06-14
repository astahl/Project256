#include "GameState.h"


TimingData GameState::timingData{
    .getPlatformTimeMicroseconds = []() {
        LARGE_INTEGER frequency, ticks;
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&ticks);
        return (ticks.QuadPart * 1'000'000) / frequency.QuadPart;
    }
};

GameState::GameState() {
    this->memory = reinterpret_cast<byte*>(VirtualAlloc(LPVOID(2LL * 1024 * 1024 * 1024), MemorySize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    this->drawBuffer = reinterpret_cast<byte*>(VirtualAlloc(LPVOID(2LL * 1024 * 1024 * 1024 + MemorySize), 4 * DrawBufferHeight * DrawBufferWidth, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
}

GameOutput GameState::tick() {
    profiling_time_interval(&GameState::timingData, eTimerTickToTick, eTimingTickToTick);
    profiling_time_set(&GameState::timingData, eTimerTickToTick);

    profiling_time_set(&GameState::timingData, eTimerTick);
    input.hasMouse = true;

    input.frameNumber = frameCount++;
    auto theframeTime = frameTime.elapsed();
    upTime += theframeTime.microseconds;
    input.elapsedTime_s = theframeTime.seconds;
    input.upTime_microseconds = upTime;;

    GameInput inputCopy = input;
    input = {};
    GameOutput output{};

    profiling_time_interval(&GameState::timingData, eTimerTick, eTimingTickSetup);
    output = doGameThings(&inputCopy, memory);
    profiling_time_interval(&GameState::timingData, eTimerTick, eTimingTickDo);


    if (inputCopy.mouse.trackLength && inputCopy.mouse.endedOver) {
        input.mouse.track[0] = inputCopy.mouse.track[inputCopy.mouse.trackLength - 1];
        input.mouse.trackLength += 1;
        input.mouse.endedOver = true;
    }
    input.mouse.buttonLeft.endedDown = inputCopy.mouse.buttonLeft.endedDown;
    input.mouse.buttonRight.endedDown = inputCopy.mouse.buttonRight.endedDown;
    input.mouse.buttonMiddle.endedDown = inputCopy.mouse.buttonMiddle.endedDown;
    return output;
}