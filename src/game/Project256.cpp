#define CXX
#include "Project256.h"
#include <cstdint>
#include "Drawing/Palettes.cpp"
#include <iostream>
#include <chrono>

struct Timer {
    std::chrono::microseconds firesAt;

    Timer(std::chrono::microseconds currentTime, std::chrono::microseconds period)
    : firesAt{currentTime + period}{
    }

    bool hasFired(std::chrono::microseconds currentTime) const {
        return currentTime > firesAt;
    }
};

template<typename T, typename U1, typename U2>
constexpr T wrapAround(T a, U1 lowerBound, U2 upperBound) {
    while (a >= upperBound) {
        a -= upperBound;
    }
    while (a <= lowerBound) {
        a += upperBound;
    }
    return a;
}

struct GameMemory {
    uint8_t vram[DrawBufferWidth * DrawBufferHeight];
    uint32_t palette[16];
    Vec2f dotPosition;
    Vec2f dotDirection;
    Timer directionChangeTimer;
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
    GameMemory& memory = *reinterpret_cast<GameMemory*>(pMemory);
    GameInput& input = *pInput;
    if (input.textLength)
        std::cout << input.text_utf8;
    const auto time = std::chrono::microseconds(input.upTime_microseconds);

    if (input.frameNumber == 0) {
        for(int i = 0; i < DrawBufferWidth * DrawBufferHeight; ++i)
            memory.vram[i] = i % Palette::count;
//        PaletteCGA::writeTo(memory.palette, PaletteCGA::Mode::Mode5LowIntensity, PaletteCGA::Color::darkGray);
        Palette::writeTo(memory.palette);
        memory.dotPosition = Vec2f{DrawBufferWidth / 2, DrawBufferHeight / 2};
    }

    if (memory.directionChangeTimer.hasFired(time)) {
        memory.directionChangeTimer = Timer(time, std::chrono::seconds(1));
        memory.dotDirection.x = static_cast<float>((rand() % 100) - 50) / 1.0f;
        memory.dotDirection.y = static_cast<float>((rand() % 100) - 50) / 1.0f;
    }

    if (input.hasMouse) {
        if (input.mouse.trackLength) {
            Vec2f mousePosition = input.mouse.track[input.mouse.trackLength - 1];

            Vec2i position{
                .x = static_cast<int>(mousePosition.x),
                .y = static_cast<int>(mousePosition.y),
            };

            if (position.x < DrawBufferWidth && position.x >= 0 &&
                position.y < DrawBufferHeight && position.y >= 0)
                memory.vram[position.x + position.y * DrawBufferWidth] = position.x % 15 + 1;

        } else {

           // memory.mousePosition = Vec2f{-1.0f, -1.0f};
        }
    }
    // clear
    memory.vram[static_cast<int>(memory.dotPosition.x) + static_cast<int>(memory.dotPosition.y) * DrawBufferWidth] = 0x0;
    // update
    memory.dotPosition.x += static_cast<float>(input.elapsedTime_s * memory.dotDirection.x);
    memory.dotPosition.y += static_cast<float>(input.elapsedTime_s * memory.dotDirection.y);

    memory.dotPosition.x = wrapAround(memory.dotPosition.x, 0, DrawBufferWidth);
    memory.dotPosition.y = wrapAround(memory.dotPosition.y, 0, DrawBufferHeight);

    // draw
    memory.vram[static_cast<int>(memory.dotPosition.x) + static_cast<int>(memory.dotPosition.y) * DrawBufferWidth] = 0x1;

	return GameOutput{
		.shouldQuit = input.closeRequested,
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
            *pixel++ = 0xFF000000 | ((y % 2) && (x % 2) ? 0xFFFFFF : 0x000000) ; // argb
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
