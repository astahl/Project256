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
#include "Utility/Flags.hpp"

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
    HiddenMine = Mine | HiddenFlag,
    FlaggedMine = HiddenMine | FlaggedFlag,
};

template<typename T, T flag>
requires(std::is_enum_v<T>)
bool testFlag(const T value) {
    using underlying_t = std::underlying_type_t<T>;
    return (static_cast<underlying_t>(value) & static_cast<underlying_t>(flag)) == static_cast<underlying_t>(flag);
}

template<typename T, T flag>
requires(std::is_enum_v<T>)
void setFlag(T& value) {
    using underlying_t = std::underlying_type_t<T>;
    value = static_cast<T>(static_cast<underlying_t>(value) | static_cast<underlying_t>(flag));
}

template<typename T, T flag>
requires(std::is_enum_v<T>)
void unsetFlag(T& value) {
    using underlying_t = std::underlying_type_t<T>;
    value = static_cast<T>(static_cast<underlying_t>(value) & ~static_cast<underlying_t>(flag));
}

template<typename T, T flag>
requires(std::is_enum_v<T>)
bool toggleFlag(T& value) {
    using underlying_t = std::underlying_type_t<T>;
    bool isSet = testFlag<T, flag>(value);
    if (isSet) {
        unsetFlag<T, flag>(value);
    }
    else {
        setFlag<T, flag>(value);
    }
    return !isSet;
}


enum class GameState {
    Init,
    Menu,
    Play,
    Pause,
    Lose,
    Win,
};

const auto MINECOUNT = 40;
const auto WIDTH = 16;
const auto HEIGHT = 16;

using GameBoard_t = Image<CellState, WIDTH, HEIGHT>;

const auto CHARACTER_WIDTH = 8;
const auto CHARACTER_HEIGHT = 8;
const auto CHARACTER_MAP_SIZE = 256;

using CharROM = CharacterRom::PET;

struct ColorIndexPair {
    uint8_t foreground;
    uint8_t background;
};


template <int NCols, int NLines, int CharW, int CharH>
struct Screen
{
    constant int ColumnCount = NCols;
    constant int LineCount = NLines;
    constant int CharacterWidth = CharW;
    constant int CharacterHeight = CharH;
    using Text_t = uint8_t;
    using TextBuffer_t = Image<Text_t, ColumnCount, LineCount, ImageOrigin::TopLeft>;
    using ColorBuffer_t = Image<ColorIndexPair, ColumnCount, LineCount, ImageOrigin::TopLeft>;
    std::array<BitmapImage<CharacterWidth, CharacterHeight>, CHARACTER_MAP_SIZE> characters;
    TextBuffer_t buffer;
    ColorBuffer_t color;
    Vec2i marker;
    bool isDirty;
    bool showMarker;
};

using Screen_t = Screen<DrawBufferWidth / CHARACTER_WIDTH, DrawBufferHeight / CHARACTER_HEIGHT, CHARACTER_WIDTH, CHARACTER_HEIGHT>;

void print(Screen_t& screen, const std::u32string_view& str, ranges_at_home::aRangeOf<Vec2i> auto positions, std::optional<ColorIndexPair> color = std::nullopt) {
    for (auto [character, position] : ranges_at_home::zip(str, positions))
    {
        if (isValidImageIndex(screen.buffer, position)) {
            screen.buffer.at(position) = CharacterRom::PET::CharacterForCodepoint(character).value_or(static_cast<Screen_t::Text_t>(CharacterRom::PET::SpecialCharacters::Bullet));
            if (color.has_value()) {
                screen.color.at(position) = color.value();
            }
        }
    }
}

void print(Screen_t& screen, const std::string_view& str, ranges_at_home::aRangeOf<Vec2i> auto positions, std::optional<ColorIndexPair> color = std::nullopt) {
    for (auto [character, position] : ranges_at_home::zip(str, positions))
    {
        if (isValidImageIndex(screen.buffer, position)) {
            screen.buffer.at(position) = CharacterRom::PET::CharacterForCodepoint(character).value_or(static_cast<Screen_t::Text_t>(CharacterRom::PET::SpecialCharacters::Bullet));
            if (color.has_value()) {
                screen.color.at(position) = color.value();
            }
        }
    }
}


void draw(const Screen_t& screen, anImageOf<uint8_t> auto& destination)
{
    using namespace ranges_at_home;
    auto line = destination.line(destination.height() - Screen_t::CharacterHeight);
    uint8_t* drawPointer = line.data();
    auto textLines = screen.buffer.linesView();
    auto colorLines = screen.color.linesView();
    for (const auto [textLine, colorLine] : zip(textLines, colorLines))
    {
        uint8_t* linePointer = drawPointer;
        for (int y = Screen_t::CharacterHeight - 1; y >= 0; --y) {
            uint64_t* dst = reinterpret_cast<uint64_t*>(linePointer);

            for (int x = 0; x < Screen_t::ColumnCount; ++x)
            {
                const auto& text = textLine[x];
                const auto& color = colorLine[x];
                const uint64_t pixels8 = spread(screen.characters[text].at({0, y}));
                const uint64_t background = ~(pixels8 * 0xFF) / 0xFF;
                *dst++ = pixels8 * color.foreground | background * color.background;
            }

            linePointer += destination.pitch();
        }
        drawPointer -= destination.pitch() * Screen_t::CharacterHeight;
    }
}

using VideoBuffer_t = Image<uint8_t, DrawBufferWidth, DrawBufferHeight>;

struct GameMemory {
    std::array<uint32_t, 256> palette;
    VideoBuffer_t videobuffer;
    bool isVideoBufferDirty;

    GameState state, previousState;
    GameBoard_t board;
    int turnCount;

    Vec2i selectedCell;

    Vec2i boardOffset;

    Screen_t screen;

    uint32_t activeControllerIndex;
    Vec2i moveSelector;
    AutoResettingTimer moveTimer;
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
        memory.board.at(minePosition) = CellState::Mine;
    }

    for (auto position : indices) {

        if (memory.board.at(position) != CellState::Free) {
            continue;
        }
        Vec2i neighborCornerMin = clamp(position - Vec2i{1,1}, originIndex, cornerIndex);
        Vec2i neighborCornerMax = clamp(position + Vec2i{1,1}, originIndex, cornerIndex);
        uint8_t mineCount = 0;
        for (auto neighborPosition : Generators::Rectangle(neighborCornerMin, neighborCornerMax)) {
            if (memory.board.at(neighborPosition) == CellState::Mine) {
                ++mineCount;
            }
        }
        memory.board.at(position) = static_cast<CellState>(mineCount);
    }

    for (auto& cell : memory.board.pixels()) {
        cell = static_cast<CellState>(static_cast<uint8_t>(cell) | static_cast<uint8_t>(CellState::HiddenFlag));
    }
}

void showBoard(const GameBoard_t& board, Screen_t& screen, Vec2i offset) {
    for (auto position : Generators::Rectangle(Vec2i{}, board.maxIndex()))
    {
        auto destination = position + offset;
        Screen_t::Text_t t;
        ColorIndexPair c = {1,0};

        auto cell = board.at(position);
        const auto flags = Flags{cell};
        using enum CellState;
        if (flags.test(FlaggedFlag)) {
            t = static_cast<Screen_t::Text_t>(CharacterRom::PET::SpecialCharacters::LineCrossDiag);
            c = {static_cast<uint8_t>(PaletteC64::Color::white), 0};
        }
        else if (flags.test(HiddenFlag)) {
            t = static_cast<Screen_t::Text_t>(CharacterRom::PET::SpecialCharacters::HalfTone);
            c = {3, 14};
        }
        else
        {
            switch (cell) {
                case CellState::Mine:
                    t = CharROM::CharacterTable['*'];
                    c = {static_cast<uint8_t>(PaletteC64::Color::red), 0}; break;
                case CellState::NextToOne:
                    t = CharROM::CharacterTable['1'];
                    c = {static_cast<uint8_t>(PaletteC64::Color::green), 0}; break;
                case CellState::NextToTwo:
                    t = CharROM::CharacterTable['2'];
                    c = {static_cast<uint8_t>(PaletteC64::Color::lightBlue), 0}; break;
                case CellState::NextToThree:
                    t = CharROM::CharacterTable['3'];
                    c = {static_cast<uint8_t>(PaletteC64::Color::yellow), 0}; break;
                case CellState::NextToFour:
                    t = CharROM::CharacterTable['4'];
                    c = {static_cast<uint8_t>(PaletteC64::Color::orange), 0}; break;
                case CellState::NextToFive:
                    t = CharROM::CharacterTable['5'];
                    c = {static_cast<uint8_t>(PaletteC64::Color::lightRed), 0}; break;
                case CellState::NextToSix:
                    t = CharROM::CharacterTable['6'];
                    c = {static_cast<uint8_t>(PaletteC64::Color::cyan), 0}; break;
                case CellState::NextToSeven:
                    t = CharROM::CharacterTable['7'];
                    c = {static_cast<uint8_t>(PaletteC64::Color::blue), 0}; break;
                case CellState::NextToEight:
                    t = CharROM::CharacterTable['8'];
                    c = {static_cast<uint8_t>(PaletteC64::Color::purple), 0}; break;
                default: t = CharROM::CharacterTable[' ']; break;
            }
        }

        screen.buffer.at(destination) = t;
        screen.color.at(destination) = c;

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

void onActionUnhideSelect(GameMemory& memory) {
    const Vec2i boardPosition = memory.selectedCell;
    if (boardPosition >= Vec2i{} && boardPosition <= memory.board.maxIndex()) {
        CellState& cell = memory.board.at(boardPosition);
        if (testFlag<CellState, CellState::HiddenFlag>(cell)) {
            unsetFlag<CellState, CellState::HiddenFlag>(cell);
            ++memory.turnCount;
            if (cell == CellState::Mine) {
                memory.state = GameState::Lose;
            }
            if (cell == CellState::Free) {
                // fill / uncover brute force whole board
                int changedCells = 1;
                while (changedCells) {
                    changedCells = 0;
                    for (auto pos : Generators::Rectangle({}, memory.board.maxIndex()))
                    {
                        auto& fillingCell = memory.board.at(pos);
                        if (!testFlag<CellState, CellState::HiddenFlag>(fillingCell)) {
                            continue;
                        }
                        auto cornerMin = max(pos - Vec2i{1,1}, Vec2i{});
                        auto cornerMax = min(pos + Vec2i{1,1}, memory.board.maxIndex());
                        for (auto neighborPos : Generators::Rectangle(cornerMin, cornerMax)) {
                            if (neighborPos == pos) {
                                continue;
                            }
                            auto& neighborCell = memory.board.at(neighborPos);
                            if (neighborCell == CellState::Free) {
                                unsetFlag<CellState, CellState::HiddenFlag>(fillingCell);
                                unsetFlag<CellState, CellState::FlaggedFlag>(fillingCell);
                                changedCells++;
                                break;
                            }
                        }
                    }
                }


            }

            auto cells = memory.board.pixels();
            if (std::all_of(cells.begin(), cells.end(), [](auto& c) {
                return !testFlag<CellState, CellState::HiddenFlag>(c) || c == CellState::HiddenMine || c == CellState::FlaggedMine;
            })) {
                memory.state = GameState::Win;
            };

            showBoard(memory.board, memory.screen, memory.boardOffset);
            memory.screen.isDirty = true;
        }
    }
}

void onActionFlagSelected(GameMemory& memory) {
    const Vec2i boardPosition = memory.selectedCell;
    if (boardPosition >= Vec2i{} && boardPosition <= memory.board.maxIndex()) {
        CellState& cell = memory.board.at(boardPosition);
        if (testFlag<CellState, CellState::HiddenFlag>(cell)) {
            toggleFlag<CellState, CellState::FlaggedFlag>(cell);
            showBoard(memory.board, memory.screen, memory.boardOffset);
            memory.screen.isDirty = true;
        }
    }
}


struct Minesweeper {


    using DrawBuffer = Image<uint32_t, DrawBufferWidth, DrawBufferHeight>;
    using AudioBuffer = Audio::PCM16StereoBuffer<AudioFramesPerBuffer>;
    using MemoryLayout = GameMemory;

    static GameOutput doGameThings(MemoryLayout& memory, const FrameInput::Input& input, const PlatformCallbacks& callbacks)
    {
        using namespace ranges_at_home;
        using namespace Generators;
        using namespace std::chrono_literals;

        constant auto color = ColorIndexPair{13,4};
        auto time = std::chrono::microseconds(input.upTime_microseconds);

        auto output = GameOutput {
            .shouldQuit = input.closeRequested,
            .shouldShowSystemCursor = true,
        };
        bool controllerChanged = false;
        for (uint32_t i = 0; i < input.controllers.size(); ++i) {
            if (input.controllers[i].isActive) {
                if (memory.activeControllerIndex != i) {
                    memory.activeControllerIndex = i;
                    controllerChanged = true;
                }
            }
        }
        const GameController& controller = input.controllers[memory.activeControllerIndex];

        bool up = controller.stickLeft.up.endedDown || controller.dPad.up.endedDown;
        bool down = controller.stickLeft.down.endedDown || controller.dPad.down.endedDown;
        bool left = controller.stickLeft.left.endedDown || controller.dPad.left.endedDown;
        bool right = controller.stickLeft.right.endedDown || controller.dPad.right.endedDown;
        auto buttonPressed = [](const auto& btn) { return btn.endedDown && btn.transitionCount; };
        bool primary = buttonPressed(controller.buttonA) || buttonPressed(input.mouse.buttonLeft) || !input.taps.empty();
        bool secondary = buttonPressed(controller.buttonB) || buttonPressed(input.mouse.buttonRight);

        bool stateWasEntered = memory.previousState != memory.state;
        memory.previousState = memory.state;
        switch(memory.state) {
            case GameState::Init:
                PaletteC64::writeTo(memory.palette.data());
                callbacks.readFile(CharROM::Filename.data(), memory.screen.characters.front().bytes(), sizeof(memory.screen.characters));
                memory.screen.buffer.fill(CharROM::CharacterTable[' ']);
                memory.screen.color.fill(color);
                memory.boardOffset = (memory.screen.buffer.size2d() - memory.board.size2d()) / 2;
                memory.screen.isDirty = true;
                memory.moveTimer = AutoResettingTimer(time, 100ms);
                // check if initialisation is done
                memory.state = GameState::Menu;
                break;
            case GameState::Menu:
                if (stateWasEntered) {
                    memory.screen.buffer.fill(CharROM::CharacterTable[' ']);
                    memory.screen.color.fill(color);

                    print(memory.screen, "NonSquPix", Generators::Rectangle({0,0}, {2,2}), ColorIndexPair{15,6});
                    print(memory.screen, "areels", Generators::Rectangle({3,1}, {5,2}), ColorIndexPair{4,14});

                    memory.screen.isDirty = true;
                }
                if (primary) {
                    // check if user selected start game
                    memory.state = GameState::Play;
                    resetGame(memory);
                    memory.screen.buffer.fill(CharROM::CharacterTable[' ']);
                    memory.turnCount = 0;
                    memory.selectedCell = Vec2i();
                    memory.screen.marker = memory.selectedCell + memory.boardOffset;
                    memory.screen.showMarker = true;

                    showBoard(memory.board, memory.screen, memory.boardOffset);
                    memory.screen.isDirty = true;
                }
                break;
            case GameState::Play:
            {
                auto mouseOverBoardPos = Vec2i{};
                if (input.mouse.endedOver && input.mouse.track.size() > 1) {
                    auto mousePos = input.mouse.track.back();
                    auto screenBufferPos = mapPositions(mousePos, memory.videobuffer, memory.screen.buffer);
                    mouseOverBoardPos = screenBufferPos - memory.boardOffset;
                    if (mouseOverBoardPos >= Vec2i{} && mouseOverBoardPos <= memory.board.maxIndex()) {
                        output.shouldShowSystemCursor = false;
                    }
                    else {
                        output.shouldShowSystemCursor = true;
                    }
                    memory.selectedCell = mouseOverBoardPos + Vec2i{0,1};
                }
                else if (!input.taps.empty()) {
                    auto tapPos = input.taps.back().position;
                    auto screenBufferPos = mapPositions(tapPos, memory.videobuffer, memory.screen.buffer);
                    mouseOverBoardPos = screenBufferPos - memory.boardOffset;
                    memory.selectedCell = mouseOverBoardPos + Vec2i{0,1};
                }
                else if (memory.moveTimer.hasFired(time)) {
                    if (up) {
                        memory.selectedCell.y -= 1;
                    }
                    if (down) {
                        memory.selectedCell.y += 1;
                    }
                    if (left) {
                        memory.selectedCell.x -= 1;
                    }
                    if (right) {
                        memory.selectedCell.x += 1;
                    }

                }
                
                memory.selectedCell = clamp(memory.selectedCell, Vec2i{}, memory.board.maxIndex());
                memory.screen.marker = memory.selectedCell + memory.boardOffset;
                memory.screen.showMarker = true;
                memory.screen.isDirty = true;

                // back button
                if (buttonPressed(controller.buttonBack)) {
                    memory.state = GameState::Menu; break;
                }


//                memory.moveSelector = round(controller.stickLeft.end);
//
//                if (memory.moveTimer.hasFired(time)) {
//                    memory.selectedCell = memory.selectedCell + memory.moveSelector;
//                    memory.screen.marker = memory.selectedCell + memory.boardOffset;
//                    memory.screen.isDirty = true;
//                }
                if (primary) {
                    onActionUnhideSelect(memory);
                }
                if (secondary) {
                    onActionFlagSelected(memory);
                }

                break;
            }
            case GameState::Pause: break;
            case GameState::Lose:
                if (stateWasEntered) {
                    for (auto& cell : memory.board.pixels()) {
                        if (cell == CellState::HiddenMine) {
                            cell = CellState::Mine;
                        }
                    }


                    std::array<char, 128> buffer;
                    size_t count = snprintf(buffer.data(), 128, "You lost after %d turns", memory.turnCount);
                    std::string_view sv{buffer.data(), count};

                    print(memory.screen, sv, Generators::Rectangle({0,1}, {10,3}));
                    showBoard(memory.board, memory.screen, memory.boardOffset);
                    memory.screen.isDirty = true;
                }
                if (primary) {
                    memory.state = GameState::Menu;
                }
                break;
            case GameState::Win:
                if (primary) {
                    memory.state = GameState::Menu;
                }
                break;
        }

        if (memory.screen.isDirty) {
            draw(memory.screen, memory.videobuffer);
            memory.screen.isDirty = false;
            memory.isVideoBufferDirty = true;
        }

        if (memory.screen.showMarker) {
            auto markerPosition = mapPositions(memory.screen.marker, memory.screen.buffer, memory.videobuffer);
            for (auto pix :
                 concat(
                     concat(
                         HLine(markerPosition, CHARACTER_WIDTH), 
                         alternate(
                             VLine(markerPosition, -CHARACTER_HEIGHT),
                             VLine(markerPosition + Vec2i{CHARACTER_WIDTH - 1, 0}, -CHARACTER_HEIGHT))),
                    HLine(markerPosition + Vec2i{0, 1 - CHARACTER_HEIGHT}, CHARACTER_WIDTH))) {
                if (pix >= Vec2i{} && pix < memory.videobuffer.size2d()) {
                    memory.videobuffer.at(pix) = 1;
                }
            }
            memory.isVideoBufferDirty = true;
        }

        return output;
    }

    static void writeDrawBuffer(MemoryLayout& memory, DrawBuffer& buffer)
    {
        if (memory.state == GameState::Init) {
            int colorIndex = 0;
            WebColorRGB colors[] { WebColorRGB::Aqua, WebColorRGB::WhiteSmoke, WebColorRGB::HotPink, WebColorRGB::Black };
            for (auto line : buffer.linesView())
            {
                auto lineColor = colors[colorIndex++ % 4];
                for (auto& pix : line) {
                    pix = 0xFF000000 | static_cast<ColorARGB>(lineColor);
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

    static void writeAudioBuffer(MemoryLayout& memory, AudioBuffer& buffer, const AudioBufferDescriptor& /*bufferDescriptor*/)
    {
        buffer.clear();
       // printf("%lf\n", bufferDescriptor.sampleTime / bufferDescriptor.sampleRate);
        memory = memory;
    }

};

