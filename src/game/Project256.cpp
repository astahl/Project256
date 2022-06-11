#define CXX
#include "Project256.h"
#include <cstdint>
#include "Drawing/Palettes.cpp"
#include "Drawing/Generators.hpp"
#include "Math/Vec2Math.hpp"
#include <iostream>
#include <chrono>

struct Timer {
    std::chrono::microseconds firesAt;

    Timer(std::chrono::microseconds currentTime, std::chrono::microseconds period)
    : firesAt{currentTime}{
        firesAt += period;
    }

    bool hasFired(std::chrono::microseconds currentTime) const {
        return currentTime > firesAt;
    }
};


template<typename Color, typename Vec2 = Vec2i, int Pitch = DrawBufferWidth>
constexpr void put(uint8_t* drawBuffer, Vec2 position, Color color)
{
    drawBuffer[static_cast<int>(position.x) + static_cast<int>(position.y) * Pitch] = static_cast<uint8_t>(color);
}


struct GameMemory {
    uint8_t vram[DrawBufferWidth * DrawBufferHeight];
    uint32_t palette[16];
    Vec2f dotPosition;
    Vec2f dotDirection;
    Timer directionChangeTimer;
    Vec2i points[2];
    int currentPoint;
};

static_assert(sizeof(GameMemory) <= MemorySize, "MemorySize is too small to hold GameMemory struct");

extern "C" {

Vec2f clipSpaceDrawBufferScale(unsigned int viewportWidth, unsigned int viewportHeight)
{
    // we COULD assert that viewport dimensions are not greater than UINT16_MAX each...
    uint32_t viewportAspectRatio16_16 = (viewportWidth << 16) / viewportHeight;
    uint32_t drawAspectRatio16_16 = (DrawAspectH << 16) / DrawAspectV;

    uint32_t widthOnTarget = viewportWidth;
    uint32_t heightOnTarget = viewportHeight;

    if (viewportAspectRatio16_16 > drawAspectRatio16_16) {
        // window is wider than the draw target, shrink on x
        widthOnTarget = (heightOnTarget * drawAspectRatio16_16) >> 16;
    }
    else if (drawAspectRatio16_16 > viewportAspectRatio16_16) {
        heightOnTarget = (widthOnTarget << 16) / drawAspectRatio16_16;
    }

    return {
        .x = static_cast<float>(widthOnTarget) / viewportWidth,
        .y = static_cast<float>(heightOnTarget) / viewportHeight,
    };
}

GameOutput doGameThings(GameInput* pInput, void* pMemory)
{
    using Palette = PaletteC64;
    const auto clearColor = Palette::Color::black;
    const auto center = Vec2i{ DrawBufferWidth / 2, DrawBufferHeight / 2};
    GameMemory& memory = *reinterpret_cast<GameMemory*>(pMemory);
    GameInput& input = *pInput;
    if (input.textLength)
        std::cout << input.text_utf8;

    if (input.frameNumber == 0) {
        Palette::writeTo(memory.palette);
        memory.dotPosition = itof(center);
    }

    const auto time = std::chrono::microseconds(input.upTime_microseconds);
    if (memory.directionChangeTimer.hasFired(time)) {
        memory.directionChangeTimer = Timer(time, std::chrono::seconds(2));
        memory.dotDirection = itof((rand2d() % 100) - Vec2i{50, 50});
    }

    std::memset(memory.vram, (uint8_t)clearColor, DrawBufferWidth * DrawBufferHeight);
    auto wrap = [](auto p) { return wrapAround2d(p, Vec2i(), Vec2i{DrawBufferWidth, DrawBufferHeight});};
    if (input.hasMouse) {
        if (input.mouse.trackLength) {
            Vec2f mousePosition = input.mouse.track[input.mouse.trackLength - 1];

            Vec2i position = truncate(mousePosition);

            if (input.mouse.buttonLeft.endedDown) {
                memory.points[memory.currentPoint] = position;
            } else if (input.mouse.buttonLeft.transitionCount) {
                memory.currentPoint = (memory.currentPoint + 1) % 2;
            }

            if (input.mouse.buttonLeft.endedDown) {
                for (auto p : Generators::Rectangle{ .bottomLeft = position - Vec2i{3,3}, .topRight = position + Vec2i{3, 3} })
                    put(memory.vram, wrap(p), Palette::Color::red);

            }

            for (auto p : Generators::Rectangle{ .bottomLeft = position - Vec2i{3,0}, .topRight = position + Vec2i{3, 0} })
                put(memory.vram, wrap(p), Palette::Color::white);
            for (auto p : Generators::Rectangle{ .bottomLeft = position - Vec2i{0,3}, .topRight = position + Vec2i{0, 3} })
                put(memory.vram, wrap(p), Palette::Color::white);
        }
    }

    for (auto p : Generators::Line{memory.points[0], memory.points[1]})
        put(memory.vram, p, Palette::Color::white);

    // update
    memory.dotPosition = memory.dotPosition + static_cast<float>(input.elapsedTime_s) * memory.dotDirection;
    memory.dotPosition = wrap(memory.dotPosition);

    // draw
    put(memory.vram, memory.dotPosition, Palette::Color::lightRed);

	return GameOutput{
		.shouldQuit = input.closeRequested,
        .shouldPinMouse = input.mouse.buttonRight.endedDown,
        .shouldShowSystemCursor = input.mouse.buttonMiddle.endedDown,
	};
}

void writeDrawBuffer(void* pMemory, void* buffer)
{
    if (buffer == nullptr) {
        return;
    }
    uint32_t* pixel = reinterpret_cast<uint32_t*>(buffer);

    if (pMemory == nullptr) {
        for (unsigned y = 0; y < DrawBufferHeight; ++y)
        for (unsigned x = 0; x < DrawBufferWidth; ++x)
        {
            *pixel++ = 0xFF000000 | ((y % 2) && (x % 2) ? 0xFFFFFF : y) ; // argb
        }
        return;
    }

    GameMemory& memory = *reinterpret_cast<GameMemory*>(pMemory);
    uint8_t* vram = memory.vram;
    
    for (unsigned y = 0; y < DrawBufferHeight; ++y)
    for (unsigned x = 0; x < DrawBufferWidth; ++x)
        *pixel++ = memory.palette[*vram++];
}

}
