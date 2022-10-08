#pragma once

#include "Project256.h"
#include <array>

template<typename T, size_t C>
struct DynamicBuffer {
    using ContainerT = std::array<T, C>;
    using IteratorT = typename ContainerT::iterator;
    using ConstIteratorT = typename ContainerT::const_iterator;

	unsigned length;
	ContainerT data;

    ConstIteratorT begin() const {
        return data.begin();
    }

    ConstIteratorT end() const {
        return data.begin() + length;
    }

    IteratorT begin() {
        return data.begin();
    }

    IteratorT end() {
        return data.begin() + length;
    }
};


struct FrameInput {
    double elapsedTime_s;
    long long upTime_microseconds;
    long long unsigned frameNumber;

    DynamicBuffer<GameController, InputMaxControllers> controllers;
    DynamicBuffer<Tap, InputMaxTaps> taps;
    DynamicBuffer<char8_t, InputMaxTextLength> text;
    
    struct Mouse mouse;

    _Bool closeRequested;
};


static_assert(sizeof(FrameInput) == sizeof(GameInput));