#pragma once

#include "Project256.h"
#include <array>
#include <chrono>
#include <span>

template<typename T, size_t C>
struct DynamicBuffer {
    using ContainerT = std::array<T, C>;
    using ElementT = T;
    using ConstElementT = const T;
    using ReferenceT = T&;
    using ConstReferenceT = const T&;
    using IteratorT = typename ContainerT::iterator;
    using ConstIteratorT = typename ContainerT::const_iterator;

	unsigned length;
	ContainerT data;

    constexpr ConstIteratorT begin() const {
        return data.begin();
    }

    constexpr ConstIteratorT end() const {
        return data.begin() + length;
    }

    constexpr IteratorT begin() {
        return data.begin();
    }

    constexpr IteratorT end() {
        return data.begin() + length;
    }

    constexpr size_t size() const {
        return length;
    }

    constexpr bool empty() const {
        return length == 0;
    }

    constexpr ReferenceT operator[](int i) {
        return data[i];
    }

    constexpr ConstReferenceT operator[](int i) const {
        return data[i];
    }

    constexpr ReferenceT front() {
        return data[0];
    }

    constexpr ConstReferenceT front() const {
        return data[0];
    }

    constexpr ReferenceT back() {
        return data[length - 1];
    }

    constexpr ConstReferenceT back() const {
        return data[length - 1];
    }

    constexpr size_t capacity() const {
        return C;
    }

    constexpr std::span<ConstElementT> span() const {
        auto span = std::span(data);
        return span.subspan(0, length);
    }

    constexpr std::span<ElementT> span() {
        auto span = std::span(data);
        return span.subspan(0, length);
    }
};


namespace FrameInput {

struct Mouse {
    Button buttonLeft, buttonRight, buttonMiddle;
    DynamicBuffer<Vec2f, InputMouseMaxTrackLength> track;
    _Bool endedOver;
    struct Vec2f relativeMovement;
    struct Vec2f scroll;
};

struct Input {
    std::chrono::duration<double> elapsedTime_s;
    long long upTime_microseconds;
    uint64_t frameNumber;

    DynamicBuffer<GameController, InputMaxControllers> controllers;
    DynamicBuffer<Tap, InputMaxTaps> taps;
    DynamicBuffer<char8_t, InputMaxTextLength> text;
    
    struct FrameInput::Mouse mouse;

    _Bool closeRequested;
};


static_assert(sizeof(Input) == sizeof(GameInput));

}
