#include "GameState.h"


TimingData GameState::timingData{
    .getPlatformTimeMicroseconds = []() {
        LARGE_INTEGER frequency, ticks;
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&ticks);
        return (ticks.QuadPart * 1'000'000) / frequency.QuadPart;
    }
};

void PlatformInput::pushKeyEvent(WindowsKeyEvent keyEvent)
{
    constant byte SC_LCtrl = 0x1D;
    constant byte SC_RAlt = 0x38;
    constant bool CheckForAltGr = true;
    int insertPosition = this->keyEventCount;
    if constexpr (CheckForAltGr) {
   
        // AltGr is chord of LCtrl + RAlt, which is less than useful for keyboard as controller
        // We need to look into buffer if previous key is lctrl in the same direction and overwrite it.

        if (insertPosition > 0 && keyEvent.isExtended && keyEvent.scanCode == SC_RAlt) {
            WindowsKeyEvent& previous = this->keyEvents[insertPosition - 1];
            if (previous.scanCode == SC_LCtrl && keyEvent.isUp == previous.isUp) {
                insertPosition -= 1;
            }
        }
    }

    this->keyEvents[insertPosition] = keyEvent;
}


GameState::GameState() {
    this->memory = reinterpret_cast<byte*>(VirtualAlloc(LPVOID(2LL * 1024 * 1024 * 1024), MemorySize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    this->drawBuffer = reinterpret_cast<byte*>(VirtualAlloc(LPVOID(2LL * 1024 * 1024 * 1024 + MemorySize), 4 * DrawBufferHeight * DrawBufferWidth, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
}

GameOutput GameState::tick() {
    profiling_time_interval(&GameState::timingData, eTimerTickToTick, eTimingTickToTick);
    profiling_time_set(&GameState::timingData, eTimerTickToTick);

    profiling_time_set(&GameState::timingData, eTimerTick);

    input.frameNumber = platform.frameCount++;
    auto theframeTime = frameTime.elapsed();
    platform.upTime += theframeTime.microseconds;
    input.elapsedTime_s = theframeTime.seconds;
    input.upTime_microseconds = platform.upTime;

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