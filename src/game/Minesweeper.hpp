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

#include <variant>

enum class CellState : uint8_t {
    Free,
    NextToOne,
    NextToTwo,
    NextToThree,
    NextToFour,
    NextToFive,
    NextToSix,
    NextToSeven,
    NextToEight,
    Mine,
    FlaggedFlag = 1 << 6,
    HiddenFlag = 1 << 7,
};

enum class GameState {
    Init,
    Menu,
    Play,
    Pause,
    Lose,
    Win,
};

struct GameMemory {
    GameState state, previousState;
    Image<CellState, 16, 16> board;
};



struct Minesweeper {

    using DrawBuffer = Image<uint32_t, DrawBufferWidth, DrawBufferHeight>;
    using AudioBuffer = Audio::PCM16StereoBuffer<AudioFramesPerBuffer>;
    using MemoryLayout = GameMemory;

    static GameOutput doGameThings(MemoryLayout& memory, const GameInput& input, const PlatformCallbacks& callbacks)
    {
        const bool isStateEntered = memory.state != memory.previousState;
        memory.previousState = memory.state;
        switch(memory.state) {
            case GameState::Init:
                // check if initialisation is done
                memory.state = GameState::Menu;
                break;
            case GameState::Menu: break;
            case GameState::Play: break;
            case GameState::Pause: break;
            case GameState::Lose: break;
            case GameState::Win: break;
        }
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

