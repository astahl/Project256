//
//  Waves.h
//  Project256
//
//  Created by Andreas Stahl on 11.09.22.
//

#pragma once
#include <array>
#include <cstdint>

namespace Audio {

template <typename SampleType, size_t ChannelCount>
struct Frame {
    using Sample = SampleType;
    std::array<Sample, ChannelCount> samples;
};


template <typename FrameType, size_t FrameCount>
struct Wave {
    using Frame = FrameType;
    using Sample = typename FrameType::Sample;
    std::array<Frame, FrameCount> frames;

    void clear() {
        memset(frames.data(), 0, sizeof(Frame) * FrameCount);
    }
};

template <size_t FrameCount>
struct PCM16StereoBuffer : Wave<Frame<int16_t, 2>, FrameCount> {};


};
