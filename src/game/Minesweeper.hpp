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
#include "Drawing/Generators.hpp"
#include "Audio/Waves.hpp"
#include "Math/Vec2Math.hpp"
#include "FML/RangesAtHome.hpp"
#include "Utility/Text.hpp"

#include <array>
#include <random>
#include <algorithm>

enum class CellState : uint8_t {
    Free,
    NextToOne = 1,
    NextToTwo = 2,
    NextToThree = 3,
    NextToFour = 4,
    NextToFive = 5,
    NextToSix = 6,
    NextToSeven = 7,
    NextToEight = 8,
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

const auto MINECOUNT = 32;
const auto WIDTH = 16;
const auto HEIGHT = 16;

const auto CHARACTER_WIDTH = 8;
const auto CHARACTER_HEIGHT = 8;
const auto CHARACTER_MAP_SIZE = 256;
const auto SCREEN_WIDTH = DrawBufferWidth / CHARACTER_WIDTH;
const auto SCREEN_HEIGHT = DrawBufferHeight / CHARACTER_HEIGHT;

struct ColorIndexPair {
    uint8_t foreground;
    uint8_t background;
};

using GameBoard_t = Image<CellState, WIDTH, HEIGHT>;

struct Screen
{
    using Text_t = uint8_t;
    using TextBuffer_t = Image<Text_t, SCREEN_WIDTH, SCREEN_HEIGHT>;
    using ColorBuffer_t = Image<ColorIndexPair, SCREEN_WIDTH, SCREEN_HEIGHT>;
    BitmapImage<CHARACTER_WIDTH, CHARACTER_HEIGHT * CHARACTER_MAP_SIZE> characters;
    TextBuffer_t buffer;
    ColorBuffer_t color;
    Vec2i marker;
    bool isDirty;
    bool showMarker;
};

using VideoBuffer_t = Image<uint8_t, DrawBufferWidth, DrawBufferHeight>;

struct GameMemory {
    std::array<uint32_t, 256> palette;
    VideoBuffer_t videobuffer;
    bool isVideoBufferDirty;

    GameState state, previousState;
    GameBoard_t board;
    Vec2i selectedCell;

    Screen screen;
};

void resetGame(GameMemory& memory) {
    memory.selectedCell = {};
    memory.board.fill(CellState::Free);

    auto originIndex = Vec2i{0,0};
    auto cornerIndex = memory.board.maxIndex();

    auto indices = (Generators::Rectangle(originIndex, cornerIndex) | ranges_at_home::toArray<WIDTH * HEIGHT>())();

    std::array<Vec2i, MINECOUNT> minePositions;
    std::sample(indices.begin(), indices.end(), minePositions.begin(),
                MINECOUNT, std::mt19937{std::random_device{}()});

    for (auto minePosition : minePositions) {
        memory.board.pixel(minePosition) = CellState::Mine;
    }

    for (auto position : indices) {

        if (memory.board.pixel(position) != CellState::Free) {
            continue;
        }
        Vec2i neighborCornerMin = clamp(position - Vec2i{1,1}, originIndex, cornerIndex);
        Vec2i neighborCornerMax = clamp(position + Vec2i{1,1}, originIndex, cornerIndex);
        uint8_t mineCount = 0;
        for (auto neighborPosition : Generators::Rectangle(neighborCornerMin, neighborCornerMax)) {
            if (memory.board.pixel(neighborPosition) == CellState::Mine) {
                ++mineCount;
            }
        }
        memory.board.pixel(position) = static_cast<CellState>(mineCount);
    }

    for (auto& cell : memory.board.pixels()) {
        cell = static_cast<CellState>(static_cast<uint8_t>(cell) | static_cast<uint8_t>(CellState::HiddenFlag));
    }
}

void showBoard(const GameBoard_t& board, Screen& screen, Vec2i offset) {
    for (auto position : Generators::Rectangle(Vec2i{}, board.maxIndex()))
    {
        auto destination = position + offset;
        Screen::Text_t t;

        auto cell = board.at(position);
        if ((static_cast<uint8_t>(cell) & static_cast<uint8_t>(CellState::FlaggedFlag)) == static_cast<uint8_t>(CellState::FlaggedFlag)) {
            t = static_cast<Screen::Text_t>(CharacterRom::PET::SpecialCharacters::LineCrossDiag);
        }
        else if ((static_cast<uint8_t>(cell) & static_cast<uint8_t>(CellState::HiddenFlag)) == static_cast<uint8_t>(CellState::HiddenFlag)) {
            t = 0xff;
        }
        else
        {
            switch (cell) {
                case CellState::Mine: t = static_cast<Screen::Text_t>(CharacterRom::PET::SpecialCharacters::Bullet); break;
                case CellState::NextToOne: t = CharacterRom::PET::CharacterTable['1']; break;
                case CellState::NextToTwo: t = CharacterRom::PET::CharacterTable['2']; break;
                case CellState::NextToThree: t = CharacterRom::PET::CharacterTable['3']; break;
                case CellState::NextToFour: t = CharacterRom::PET::CharacterTable['4']; break;
                case CellState::NextToFive: t = CharacterRom::PET::CharacterTable['5']; break;
                case CellState::NextToSix: t = CharacterRom::PET::CharacterTable['6']; break;
                case CellState::NextToSeven: t = CharacterRom::PET::CharacterTable['7']; break;
                case CellState::NextToEight: t = CharacterRom::PET::CharacterTable['8']; break;
                default: t = CharacterRom::PET::CharacterTable[' ']; break;
            }
        }

        screen.buffer.at(destination) = t;
        screen.color.at(destination) = {2, 0};

    }
}

template <typename T, typename U> requires aStaticImage<T> && aStaticImage<U>
Vec2i mapPositions(auto position, const T& imageFrom, const U& imageTo) {
    if constexpr (T::LineOrder == U::LineOrder) {
        return {
            static_cast<int>(position.x * imageTo.width() / imageFrom.width()),
            static_cast<int>(position.y * imageTo.height() / imageFrom.height())
        };
    } else {
        return {
            static_cast<int>(position.x * imageTo.width() / imageFrom.width()),
            static_cast<int>(imageTo.height() - 1 - (position.y * imageTo.height() / imageFrom.height()))
        };
    }
}

struct Minesweeper {

    using DrawBuffer = Image<uint32_t, DrawBufferWidth, DrawBufferHeight>;
    using AudioBuffer = Audio::PCM16StereoBuffer<AudioFramesPerBuffer>;
    using MemoryLayout = GameMemory;

    static GameOutput doGameThings(MemoryLayout& memory, const GameInput& input, const PlatformCallbacks& callbacks)
    {
        memory.previousState = memory.state;
        const Vec2i boardOffset{3,3};
        switch(memory.state) {
            case GameState::Init:
                PaletteVGA::writeTo(memory.palette.data());
                callbacks.readFile(CharacterRom::PET::Filename.data(), memory.screen.characters.bytes(), memory.screen.characters.bytesSize());
                //memory.screen.buffer.fill(0xe9);
                memory.screen.buffer.fill(CharacterRom::PET::CharacterTable[' ']);
                memory.screen.color.fill({ 0, 15 });
                memory.screen.isDirty = true;
                // check if initialisation is done
                memory.state = GameState::Menu;
                break;
            case GameState::Menu:
                // check if user selected start game
                memory.state = GameState::Play;
                resetGame(memory);
                showBoard(memory.board, memory.screen, boardOffset);
                memory.screen.isDirty = true;
                break;
            case GameState::Play:
            {

                auto boardPos = Vec2i{};
                if (input.mouse.endedOver) {
                    auto mousePos = input.mouse.track[input.mouse.trackLength - 1];
                    auto screenBufferPos = mapPositions(mousePos, memory.videobuffer, memory.screen.buffer);
                    boardPos = Vec2i{screenBufferPos.x, static_cast<int>(memory.screen.buffer.height()) - 1 - screenBufferPos.y} - boardOffset;
                    if (boardPos >= Vec2i{} && boardPos < memory.board.maxIndex()) {
                        memory.selectedCell = boardPos;
                        memory.screen.marker = screenBufferPos;
                        memory.screen.showMarker = true;
                        memory.screen.isDirty = true;
                    }
                    else {
                        memory.screen.marker = Vec2i{-1,-1};
                        memory.screen.showMarker = false;
                    }
                }

                if (boardPos >= Vec2i{0,0} && boardPos < memory.board.size2d()) {
                    memory.selectedCell = boardPos;
                }
                if (input.mouse.buttonLeft.transitionCount && !input.mouse.buttonLeft.endedDown) {
                    auto& cell = memory.board.at(memory.selectedCell);
                    cell = static_cast<CellState>(static_cast<uint8_t>(cell) & ~static_cast<uint8_t>(CellState::HiddenFlag));
                    showBoard(memory.board, memory.screen, boardOffset);
                    memory.screen.isDirty = true;

                    //memory.state = GameState::Menu;
                }
                if (input.mouse.buttonRight.transitionCount && !input.mouse.buttonRight.endedDown) {
                    auto& cell = memory.board.at(memory.selectedCell);
                    if (static_cast<uint8_t>(cell) & static_cast<uint8_t>(CellState::FlaggedFlag)) {
                        cell = static_cast<CellState>(static_cast<uint8_t>(cell) & ~static_cast<uint8_t>(CellState::FlaggedFlag));
                    } else {
                        cell = static_cast<CellState>(static_cast<uint8_t>(cell) | static_cast<uint8_t>(CellState::FlaggedFlag));

                    }
                    showBoard(memory.board, memory.screen, boardOffset);
                    memory.screen.isDirty = true;

                    //memory.state = GameState::Menu;
                }


                break;
            }
            case GameState::Pause: break;
            case GameState::Lose: break;
            case GameState::Win: break;
        }

        if (memory.screen.isDirty) {
            auto line = memory.videobuffer.line(memory.videobuffer.height() - CHARACTER_HEIGHT);
            uint8_t* drawPointer = line.data();
            auto textLines = memory.screen.buffer.linesView();
            auto colorLines = memory.screen.color.linesView();
            for (const auto [textLine, colorLine] : (textLines & colorLines))
            {
                uint8_t* linePointer = drawPointer;
                for (int y = CHARACTER_HEIGHT - 1; y >= 0; --y) {
                    uint64_t* dst = reinterpret_cast<uint64_t*>(linePointer);

                    for (const auto [t, c] : (textLine & colorLine))
                    {
                        const uint64_t pixels8 = spread(memory.screen.characters.pixel(0, t * 8 + y));
                        const uint64_t background = ~(pixels8 * 0xFF) / 0xFF;
                        *dst++ = pixels8 * c.foreground | background * c.background;
                    }
                    linePointer += memory.videobuffer.pitch();
                }
                drawPointer -= memory.videobuffer.pitch() * CHARACTER_HEIGHT;
            }
            memory.screen.isDirty = false;
            memory.isVideoBufferDirty = true;
        }

        if (memory.screen.showMarker) {
            auto markerPosition = mapPositions(memory.screen.marker, memory.screen.buffer, memory.videobuffer);
            for (auto pix :
                 Generators::HLine(markerPosition, CHARACTER_WIDTH) ^ ranges_at_home::alternate(Generators::VLine(markerPosition, CHARACTER_HEIGHT), Generators::VLine(markerPosition + Vec2i{CHARACTER_WIDTH - 1, 0}, CHARACTER_HEIGHT)) ^
                 Generators::HLine(markerPosition + Vec2i{0, CHARACTER_HEIGHT - 1}, CHARACTER_WIDTH)) {
                if (pix >= Vec2i{} && pix < memory.videobuffer.size2d()) {
                    memory.videobuffer.at(pix) = 1;
                }
            }
            memory.isVideoBufferDirty = true;
        }

        return {};
    }

    static void writeDrawBuffer(MemoryLayout& memory, DrawBuffer& buffer)
    {
        int colorIndex = 0;

        Colors colors[] { Colors::Aqua, Colors::WhiteSmoke, Colors::HotPink, Colors::Black };



        if (memory.state == GameState::Init) {
            for (auto line : buffer.linesView())
            {
                auto lineColor = colors[colorIndex++ % 4];
                for (auto& pix : line) {
                    pix = 0xFF000000 | static_cast<unsigned int>(lineColor);
                }
            }
        } else  if(memory.isVideoBufferDirty) {
            constant auto stride = sizeof(uint64_t);
            constant auto width = DrawBuffer{}.width();
            static_assert (width % stride == 0);
            constant auto height = DrawBuffer{}.height();

            const uint32_t* palette = memory.palette.data();
            uint64_t* src = reinterpret_cast<uint64_t*>(memory.videobuffer.data());
            uint64_t* dst = reinterpret_cast<uint64_t*>(buffer.data());

            constant auto dstpitch = DrawBuffer{}.pitch() / 2; // 64 bit batch = 2 x 32 bit values
            constant auto srcpitch = VideoBuffer_t{}.pitch() / 8; // 64 bit batch = 8 x 8 bit values

            for (uint32_t y = 0; y < height; ++y) {
                auto srcLine = src;
                auto dstLine = dst;
                for (uint32_t x = 0; x < width; x += stride) {
                    const uint64_t sourcePixel8 = *srcLine++;

                    dstLine[0] = static_cast<uint64_t>(palette[sourcePixel8 >> 0 & 0xff]) |
                        static_cast<uint64_t>(palette[sourcePixel8 >> 8 & 0xff]) << 32;
                    dstLine[1] = static_cast<uint64_t>(palette[sourcePixel8 >> 16 & 0xff]) |
                        static_cast<uint64_t>(palette[sourcePixel8 >> 24 & 0xff]) << 32;
                    dstLine[2] = static_cast<uint64_t>(palette[sourcePixel8 >> 32 & 0xff]) |
                        static_cast<uint64_t>(palette[sourcePixel8 >> 40 & 0xff]) << 32;
                    dstLine[3] = static_cast<uint64_t>(palette[sourcePixel8 >> 48 & 0xff]) |
                        static_cast<uint64_t>(palette[sourcePixel8 >> 56 & 0xff]) << 32;
                    dstLine += 4;
                }
                src += srcpitch;
                dst += dstpitch;
            }
            memory.isVideoBufferDirty = false;
        }
    }

    static void writeAudioBuffer(MemoryLayout& memory, AudioBuffer& buffer, const AudioBufferDescriptor& bufferDescriptor)
    {
        buffer.clear();
    }

};

