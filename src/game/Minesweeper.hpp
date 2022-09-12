//
//  Minesweeper.hpp
//  Project256
//
//  Created by Andreas Stahl on 11.09.22.
//

#pragma once
#include "defines.h"
#include "Project256.h"
#include "Drawing/Palettes.hpp"
#include "Drawing/Images.hpp"
#include "Audio/Waves.hpp"


struct Minesweeper {

using DrawBuffer = Image<uint32_t, DrawBufferWidth, DrawBufferHeight>;
using AudioBuffer = Audio::PCM16StereoBuffer<AudioFramesPerBuffer>;

struct MemoryLayout {

    enum class State {
        Menu,
        Play,
        Pause,
        Lose,
        Win,
    } state;

};

static GameOutput doGameThings(MemoryLayout& memory, const GameInput& input, const PlatformCallbacks& callbacks)
{
    return {};
}

static void writeDrawBuffer(MemoryLayout& memory, DrawBuffer& buffer)
{
    int colorIndex = 0;

    Colors colors[] { Colors::Aqua, Colors::WhiteSmoke, Colors::HotPink, Colors::Black };

    for (auto line : buffer.lines())
    {
        auto lineColor = colors[colorIndex++ % 4];
        for (auto& pix : line) {
            pix = 0xFF000000 | static_cast<unsigned int>(lineColor);
        }
    }
}

static void writeAudioBuffer(MemoryLayout& memory, AudioBuffer& buffer, const AudioBufferDescriptor& bufferDescriptor)
{
    buffer.clear();
}

};

