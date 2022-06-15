#include "Project256.h"
#include <cstdint>
#include "Drawing/Palettes.hpp"
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

void cleanInput(GameInput* input) {
    for (int i = 0; i < InputMaxControllers; ++i) {
        auto& controller = input->controllers[i];
        for (int axis1Index = 0; axis1Index < InputControllerAxis1Count; ++axis1Index)
        {
            controller.axes1[axis1Index].trigger.transitionCount = 0;
            controller.axes1[axis1Index].start = controller.axes1[axis1Index].end;
        }
        for (int axis2Index = 0; axis2Index < InputControllerAxis1Count; ++axis2Index)
        {
            controller.axes2[axis2Index].left.transitionCount = 0;
            controller.axes2[axis2Index].up.transitionCount = 0;
            controller.axes2[axis2Index].down.transitionCount = 0;
            controller.axes2[axis2Index].right.transitionCount = 0;
            controller.axes2[axis2Index].start = controller.axes2[axis2Index].end;
            if (!controller.axes2[axis2Index].latches)
                controller.axes2[axis2Index].end = Vec2f{};
        }
        for (int buttonIndex = 0; buttonIndex < InputControllerButtonCount; ++buttonIndex)
        {
            controller.buttons[buttonIndex].transitionCount = 0;
        }
    }
    auto& mouse = input->mouse;
    mouse.buttonLeft.transitionCount = 0;
    mouse.buttonRight.transitionCount = 0;
    mouse.buttonMiddle.transitionCount = 0;
    if (mouse.trackLength && mouse.endedOver) {
        mouse.track[0] = mouse.track[mouse.trackLength - 1];
        mouse.trackLength = 1;
    } else {
        mouse.trackLength = 0;
    }

    input->textLength = 0;
    input->closeRequested = false;
    input->tapCount = 0;
}

GameOutput doGameThings(GameInput* pInput, void* pMemory)
{
    using Palette = PaletteC64;
    auto clearColor = Palette::Color::black;
    if (pInput->closeRequested) {
        clearColor = Palette::Color::lightRed;
    }
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
            using Color = Palette::Color;

            auto offset = [&](Vec2i p) { return p + position; };
            auto clip = [&](Vec2i p) { return !(p < Vec2i{}) && p < bufferSize; };
            auto atMouse = transform(offset) | filter(clip);

            for (auto p : Line{{3, 0}, {-3, 0}} | atMouse )
                put(memory.vram, p, Color::white);
            for (auto p : Line{{0, -3}, {0, 3}} | atMouse )
                put(memory.vram, p, Color::white);

            auto drawAcross = (Line{{-3, -3}, {3, 3}} | atMouse | forEach([&] (const Vec2i& p) {
                put(memory.vram, p, Color::white);
            }));

            drawAcross();

            auto sum = (Line{{0, 0}, {9, 3}} | reduce<std::plus<Vec2i>, Vec2i>()).run();
            put(memory.vram, sum, Color::white);
    }

    for (unsigned int i = 0; i < input.controllerCount; ++i) {
        using namespace ranges_at_home;
        using namespace Generators;
        auto offset1 = [&](Vec2i p) { return p + Vec2i { static_cast<int>(i) * 10 + 10, 10 }; };
        for (auto p : transform_view(Line{Vec2i{}, truncate(7 * input.controllers[i].stickLeft.end)}, offset1))
            put(memory.vram, wrap(p), Palette::Color::white);
        auto offset2 = [&](Vec2i p) { return p + Vec2i { static_cast<int>(i) * 10 + 15, 10 }; };
        for (auto p : transform_view(Line{Vec2i{}, truncate(7 * input.controllers[i].stickRight.end)}, offset2))
            put(memory.vram, wrap(p), Palette::Color::white);
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
        for (unsigned int y = 0; y < DrawBufferHeight; ++y)
        for (unsigned int x = 0; x < DrawBufferWidth; ++x)
        {
            *pixel++ = 0xFF000000 | ((y % 2) && (x % 2) ? 0xFFFFFF : y) ; // argb
        }
        return;
    }

    GameMemory& memory = *reinterpret_cast<GameMemory*>(pMemory);
    uint8_t* vram = memory.vram;
    if constexpr (DrawBufferWidth % 8 == 0)
    {
        constexpr int count = DrawBufferWidth * DrawBufferHeight;
        uint64_t* src = reinterpret_cast<uint64_t*>(vram);
        uint64_t* dst = reinterpret_cast<uint64_t*>(buffer);
        const uint32_t* palette = memory.palette;
        for (int i = 0; i < count; i += 8) {
            const uint64_t sourcePixel8 = *src++;
            
            *dst = static_cast<uint64_t>(palette[sourcePixel8 >> 0 & 0xff]) | 
                static_cast<uint64_t>(palette[sourcePixel8 >> 8 & 0xff]) << 32;
            *(dst + 1) = static_cast<uint64_t>(palette[sourcePixel8 >> 16 & 0xff]) |
                static_cast<uint64_t>(palette[sourcePixel8 >> 24 & 0xff]) << 32;
            *(dst + 2) = static_cast<uint64_t>(palette[sourcePixel8 >> 32 & 0xff]) |
                static_cast<uint64_t>(palette[sourcePixel8 >> 40 & 0xff]) << 32;
            *(dst + 3) = static_cast<uint64_t>(palette[sourcePixel8 >> 48 & 0xff]) |
                static_cast<uint64_t>(palette[sourcePixel8 >> 56 & 0xff]) << 32;
            dst += 4;
        }
    }
//    else if (false) {
//        // why is this slower?!??!
//        constexpr int count = DrawBufferWidth * DrawBufferHeight;
//        struct bytes_v8 { uint8_t a, b, c, d, e, f, g, h; };
//        struct uint32_v2 { uint32_t a, b; };
//        bytes_v8* src = reinterpret_cast<bytes_v8*>(vram);
//        uint32_v2* dst = reinterpret_cast<uint32_v2*>(buffer);
//        const uint32_t* palette = memory.palette;
//        for (int i = 0; i < count; i += 8) {
//            auto src8 = *src++;
//
//            *dst = { palette[src8.a], palette[src8.b] };
//            *(dst + 1) = { palette[src8.c], palette[src8.d] };
//            *(dst + 2) = { palette[src8.e], palette[src8.f] };
//            *(dst + 3) = { palette[src8.g], palette[src8.h] };
//            dst += 4;
//        }
//    }
    else {
        for (unsigned y = 0; y < DrawBufferHeight; ++y)
        for (unsigned x = 0; x < DrawBufferWidth; ++x)
            *pixel++ = memory.palette[*vram++];
    }
}

}
