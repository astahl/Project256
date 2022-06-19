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
#include <cassert>

struct Timer {
    std::chrono::microseconds firesAt;

    static Timer delay(std::chrono::microseconds currentTime, std::chrono::microseconds period)
    {
        return {.firesAt = currentTime + period };
    }

    bool hasFired(std::chrono::microseconds currentTime) const {
        return currentTime > firesAt;
    }
};


template<typename Color, typename Vec2 = Vec2i, int Pitch = DrawBufferWidth>
compiletime void put(uint8_t* drawBuffer, Vec2 position, Color color)
{
    assert(drawBuffer != nullptr);
    assert(position.x >= 0);
    assert(position.x < DrawBufferWidth);
    assert(position.y >= 0);
    assert(position.y < DrawBufferHeight);
    drawBuffer[static_cast<int>(position.x) + static_cast<int>(position.y) * Pitch] = static_cast<uint8_t>(color);
}

template<typename Return, typename ...Args>
using FunctionPointer = Return (*)(Args...);

struct GameMemory {
    std::array<uint8_t, DrawBufferWidth * DrawBufferHeight> vram;
    std::array<uint32_t, 256> palette;
    Vec2f birdPosition;
    int birdSpeed;
    Vec2i birdTarget;
    Timer directionChangeTimer;
    FunctionPointer<void, GameMemory&> timerCallback;

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


compiletime auto BufferSize = Vec2i{ DrawBufferWidth, DrawBufferHeight};
compiletime auto Center = BufferSize / 2;

void birdDirectionChange(GameMemory& memory) {
    memory.birdTarget = (rand2d() % BufferSize);
}


GameOutput doGameThings(GameInput* pInput, void* pMemory)
{
    using namespace ranges_at_home;
    using namespace Generators;

    GameOutput output{};
    using Palette = PaletteAppleII;
    compiletime uint8_t black = []() { return findNearest(Colors::Black, Palette::colors).index; }();
    compiletime uint8_t cyan = []() { return findNearest(Colors::Cyan, Palette::colors).index; }();
    compiletime uint8_t lightBlue = []() { return findNearest(Colors::LightBlue, Palette::colors).index; }();
    compiletime uint8_t red = []() { return findNearest(Colors::Red, Palette::colors).index; }();
    compiletime uint8_t white = []() { return findNearest(Colors::White, Palette::colors).index; }();
    compiletime uint8_t green = []() { return findNearest(Colors::Green, Palette::colors).index; }();

    auto clearColor = black;
    if (pInput->closeRequested) {
        clearColor = white;
    }

    GameMemory& memory = *reinterpret_cast<GameMemory*>(pMemory);
    GameInput& input = *pInput;
    if (input.textLength)
        std::cout << input.text_utf8;

    if (input.frameNumber == 0) {
        memory = GameMemory{};
        Palette::writeTo(memory.palette.data());
        memory.birdPosition = itof(Center);
        memory.timerCallback = birdDirectionChange;
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
        std::replace(memory.sprite.data.begin(), memory.sprite.data.end(), static_cast<uint8_t>(1), cyan);
        memory.currentSpriteFrame = 0;
        memory.birdSpeed = 5;
    }

    constant auto whitePixel = [&](const auto& p) { put(memory.vram.data(), p, white); };
    constant auto redPixel = [&](const auto& p) { put(memory.vram.data(), p, red); };

    const auto time = std::chrono::microseconds(input.upTime_microseconds);
    if (memory.directionChangeTimer.hasFired(time) || memory.birdTarget == round(memory.birdPosition)) {
        memory.directionChangeTimer = Timer::delay(time, std::chrono::seconds(100 / memory.birdSpeed++));
        memory.timerCallback(memory);
    }

    if (memory.spriteAnimationTimer.hasFired(time)) {
        memory.spriteAnimationTimer = Timer::delay(time, std::chrono::milliseconds(1000 / memory.birdSpeed));
        memory.currentSpriteFrame = (memory.currentSpriteFrame + 1) % decltype(memory.sprite)::frameCount;
    }

    std::memset(memory.vram.data(), (uint8_t)clearColor, DrawBufferWidth * DrawBufferHeight);

    // draw the palette in the first rows

    for (int y = 0; y < memory.palette.size() / 16; ++y) {
    for (int x = 0; x < DrawBufferWidth; ++x) {
        put(memory.vram.data(), Vec2i{ x, y }, (x * 16 / DrawBufferWidth) + y * 16);
    } }

    compiletime auto wrap = [](auto p) { return wrapAround2d(p, Vec2i(), Vec2i{DrawBufferWidth, DrawBufferHeight});};

    if (input.mouse.trackLength) {
        Vec2f mousePosition = input.mouse.track[input.mouse.trackLength - 1];
        Vec2i position = truncate(mousePosition);

        auto offset = [=](const auto& p) {
            return p + position;
        };
        compiletime auto clip = [=](const auto& p) {
            return (Vec2i{0,0} <= p) && (p < BufferSize);
        };

        if (input.mouse.buttonLeft.endedDown) {
            memory.points[memory.currentPoint] = position;
        } else if (input.mouse.buttonLeft.transitionCount) {
            memory.currentPoint = (memory.currentPoint + 1) % 2;
        }

        auto atMouse = transform(offset);
        compiletime auto clipped = filter(clip);
        compiletime auto wrapped = transform(wrap);


        compiletime auto rectangleGenerator = Rectangle{ Vec2i{-3,-3}, Vec2i{3, 3} };

        if (input.mouse.buttonLeft.endedDown) {
            for (auto p : rectangleGenerator | atMouse | clipped)
                put(memory.vram.data(), wrap(p), green);
        }

        compiletime auto crossGenerator = (Line{{3, 0}, {-3, 0}} ^ Line{{0, 3}, {0, -3}});
        compiletime auto cross = (crossGenerator | toArray<size(crossGenerator)>{}).run();
        (cross | atMouse | wrapped | forEach(whitePixel)).run();

        compiletime auto circleGenerator = Circle{.mRadius = 10};
        compiletime auto circle = (circleGenerator | toArray<size(circleGenerator)>{}).run();
        (circle | atMouse | wrapped | forEach(whitePixel)).run();

        compiletime auto ellipsisGenerator = Ellipsis{.mRadii = {30, 20}};
        compiletime auto ellipsis = (ellipsisGenerator | skip{100} | take{100} | toArray<100>{}).run();

        const auto scale = [&](Vec2i p) { return truncate(makeBase2dY(normalized(Center - mousePosition)) * itof(p)); };
        (Line{{}, {20, 0}} ^ Line{{}, {0, 40}} | transform(scale) | atMouse | wrapped | forEach(redPixel)).run();
    }

    for (int i = 0; i < InputMaxControllers; ++i) {
        auto& controller = input.controllers[i];
        if (!controller.isConnected)
            continue;

        Vec2i p{ 10, (i + 1) * 10 };
        for (auto& button : controller.buttons) {
            if (button.endedDown)
                whitePixel(p);
            p.x += 2;
        }

        for (auto& axis1 : controller.axes1) {
            if (axis1.trigger.endedDown)
                whitePixel(p);
            p.x += 2;
        }

        for (auto& axis2 : controller.axes2) {
            if (axis2.up.endedDown)
                whitePixel(p + Vec2i{ 1,1 });
            if (axis2.down.endedDown)
                whitePixel(p + Vec2i{ 1,-1 });
            if (axis2.left.endedDown)
                whitePixel(p + Vec2i{ });
            if (axis2.right.endedDown)
                whitePixel(p + Vec2i{ 2,0 });
            p.x += 4;
        }

        p.x = 10;
        p.y = (i + 1) * 10 + 3;
        if (controller.buttonBack.transitionCount != 0 && controller.buttonBack.endedDown) {
            output.shouldQuit = true;
        }
    }

    if (memory.points[0] != memory.points[1])
    for (auto p : Generators::Line{memory.points[0], memory.points[1]})
        whitePixel(p);

    // update
    memory.birdPosition = memory.birdPosition + static_cast<float>(input.elapsedTime_s) * memory.birdSpeed * normalized(itof(memory.birdTarget) - memory.birdPosition);
    memory.birdPosition = clamp(memory.birdPosition, Vec2f{}, Vec2f{DrawBufferWidth - 1, DrawBufferHeight - 1});

    put(memory.vram.data(), memory.birdTarget, lightBlue);

    // draw
    blitSprite(memory.sprite, memory.currentSpriteFrame, memory.vram.data(), DrawBufferWidth, truncate(memory.birdPosition), Vec2i{}, Vec2i{DrawBufferWidth, DrawBufferHeight});

    return output;
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
    uint8_t* vram = memory.vram.data();
    if constexpr (DrawBufferWidth % 8 == 0)
    {
        constexpr int count = DrawBufferWidth * DrawBufferHeight;
        uint64_t* src = reinterpret_cast<uint64_t*>(vram);
        uint64_t* dst = reinterpret_cast<uint64_t*>(buffer);
        const uint32_t* palette = memory.palette.data();
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
