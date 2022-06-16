#pragma once

#include <Windows.h>
#include "../game/Project256.h"

class Chronometer {
    LARGE_INTEGER frequency = {};
    int lastTimeIndex = 0;
    LARGE_INTEGER timevalues[2] = {};
public:
    struct Time {
        INT64 microseconds;
        double seconds;
    };

    Time elapsed() {
        int nextTimeIndex = (this->lastTimeIndex == 0 ? 1 : 0);
        QueryPerformanceCounter(&timevalues[nextTimeIndex]);
        auto t0 = timevalues[lastTimeIndex];
        auto t1 = timevalues[nextTimeIndex];
        this->lastTimeIndex = nextTimeIndex;
        LARGE_INTEGER elapsedMicroseconds{};
        elapsedMicroseconds.QuadPart = ((t1.QuadPart - t0.QuadPart) * 1'000'000) / frequency.QuadPart;
        return Time{
        .microseconds = elapsedMicroseconds.QuadPart,
        .seconds = double(elapsedMicroseconds.QuadPart) / 1'000'000
        };
    }

    Chronometer() {
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&timevalues[lastTimeIndex]);
    }
};


struct WindowsKeyEvent {
    WORD virtualKeyCode;
    WORD repeatCount;
    byte scanCode;
    bool isExtended;
    bool isDialogMode;
    bool isMenuMode;
    bool isAltDown;
    bool isRepeat;
    bool isUp;
};

struct PlatformInput {
    constant int MaxKeyEventCount = 16;
    WindowsKeyEvent keyEvents[MaxKeyEventCount]{};
    int keyEventCount = 0;
    uint64_t frameCount{};
    int64_t upTime{};
    Vec2i lastCursorPosition{};

    void pushKeyEvent(WindowsKeyEvent keyEvent);
};


struct GameState {
    static TimingData timingData;
    byte* memory;
    byte* drawBuffer;
    PlatformInput platform{};
    GameInput input{};
    Chronometer frameTime{};
    bool forceCursor{};

    GameState();
    GameOutput tick();
};
