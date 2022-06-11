#define CXX
#include "Project256.h"
#include <cstdint>
#include "Drawing/Palettes.cpp"
#include "Drawing/Generators.hpp"
#include "Drawing/Sprites.hpp"
#include "Math/Vec2Math.hpp"
#include "FML/RangesAtHome.hpp"
#include <iostream>
#include <chrono>
#include <algorithm>

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
    Vec2f birdPosition;
    int birdSpeed;
    Vec2i birdTarget;
    Timer directionChangeTimer;

    SpritePicture<5, 2, 4> sprite;
    Timer spriteAnimationTimer;
    int currentSpriteFrame;

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
    const auto bufferSize = Vec2i{ DrawBufferWidth, DrawBufferHeight};
    const auto center = bufferSize / 2;
    GameMemory& memory = *reinterpret_cast<GameMemory*>(pMemory);
    GameInput& input = *pInput;
    if (input.textLength)
        std::cout << input.text_utf8;

    if (input.frameNumber == 0) {
        Palette::writeTo(memory.palette);
        memory.birdPosition = itof(center);
        memory.sprite.data = {
            0, 0, 1, 0, 0,
            1, 1, 0, 1, 1,
            1, 1, 1, 1, 1,
            0, 0, 0, 0, 0,
            1, 1, 0, 1, 1,
            0, 0, 1, 0, 0,
            0, 0, 1, 0, 0,
            1, 1, 1, 1, 1,
        };
        memory.currentSpriteFrame = 0;
        memory.birdSpeed = 5;
    }

    const auto time = std::chrono::microseconds(input.upTime_microseconds);
    if (memory.directionChangeTimer.hasFired(time) || memory.birdTarget == round(memory.birdPosition)) {
        memory.directionChangeTimer = Timer(time, std::chrono::seconds(100 / memory.birdSpeed++));
        memory.birdTarget = (rand2d() % bufferSize);
    }

    if (memory.spriteAnimationTimer.hasFired(time)) {
        memory.spriteAnimationTimer = Timer(time, std::chrono::milliseconds(1000 / memory.birdSpeed));
        memory.currentSpriteFrame = (memory.currentSpriteFrame + 1) % decltype(memory.sprite)::frameCount;
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


            using namespace ranges_at_home;
            using namespace Generators;
            auto offset = [&](Vec2i p) { return p + position; };

            for (auto p : transform_view(Line{Vec2i{3, 0}, Vec2i{-3, 0}}, offset))
                put(memory.vram, wrap(p), Palette::Color::white);
            for (auto p : transform_view(Line{Vec2i{0, 3}, Vec2i{0, -3}}, offset))
                put(memory.vram, wrap(p), Palette::Color::white);
        }
    }

    for (auto p : Generators::Line{memory.points[0], memory.points[1]})
        put(memory.vram, p, Palette::Color::white);

    // update
    memory.birdPosition = memory.birdPosition + static_cast<float>(input.elapsedTime_s) * memory.birdSpeed * normalized(memory.birdTarget - memory.birdPosition);
    memory.birdPosition = clamp(memory.birdPosition, Vec2f{}, Vec2f{DrawBufferWidth - 1, DrawBufferHeight - 1});

    put(memory.vram, memory.birdTarget, Palette::Color::cyan);

    // draw
    blitSprite(memory.sprite, memory.currentSpriteFrame, memory.vram, DrawBufferWidth, truncate(memory.birdPosition), Vec2i{}, Vec2i{DrawBufferWidth, DrawBufferHeight});

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
