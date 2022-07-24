//
//  Timers.hpp
//  Project256
//
//  Created by Andreas Stahl on 03.07.22.
//

#pragma once
#include <chrono>

template<typename Return, typename ...Args>
using FunctionPointer = Return (*)(Args...);


struct Timer {
    std::chrono::microseconds firesAt;

    Timer() = default;

    Timer(std::chrono::microseconds currentTime, std::chrono::microseconds period)
    : firesAt{currentTime + period}
    {
    }

    void reset(std::chrono::microseconds currentTime, std::chrono::microseconds period) {
        firesAt = currentTime + period;
    }

    bool hasFired(std::chrono::microseconds currentTime) const {
        return currentTime > firesAt;
    }
};

struct AutoResettingTimer {
    Timer timer;
    std::chrono::microseconds period;

    AutoResettingTimer() = default;

    AutoResettingTimer(std::chrono::microseconds currentTime, std::chrono::microseconds period)
    : timer(currentTime, period), period{period} {

    }

    bool hasFired(std::chrono::microseconds currentTime) {
        if (!timer.hasFired(currentTime)) {
            return false;
        }
        timer.reset(currentTime, period);
        return true;
    }
};

struct CountdownTimer {
    AutoResettingTimer timer;
    uint64_t count;

    CountdownTimer() = default;

    CountdownTimer(std::chrono::microseconds currentTime, std::chrono::microseconds period, uint64_t count = 1)
    : timer(currentTime, period), count{count} {

    }

    bool hasFired(std::chrono::microseconds currentTime) {
        if (count == 0) {
            return false;
        }
        if (!timer.hasFired(currentTime)) {
            return false;
        }
        --count;
        return true;
    }
};
