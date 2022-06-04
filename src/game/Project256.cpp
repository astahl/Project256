#define CXX
#include "Project256.h"
#include <cstdint>

struct GameMemory {
    uint8_t vram[DrawBufferWidth * DrawBufferHeight];
    uint32_t palette[2];
    bool isInitialized;
    Vec2i mousePosition;
};

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
    GameMemory& memory = *reinterpret_cast<GameMemory*>(pMemory);
    GameInput& input = *pInput;

    if (input.frameNumber == 0) {
        for(int i = 0; i < DrawBufferWidth * DrawBufferHeight; ++i)
            memory.vram[i] = 0;
        memory.palette[0] = 0xFF006600;
        memory.palette[1] = 0xFF00FF00;
    }

    if (input.mouse.trackLength) {
        Vec2f mousePosition = input.mouse.track[input.mouse.trackLength - 1];

        Vec2i position{
            .x = static_cast<int>(mousePosition.x),
            .y = static_cast<int>(mousePosition.y),
        };

        if (position.x < DrawBufferWidth && position.x >= 0 &&
            position.y < DrawBufferHeight && position.y >= 0)
            memory.vram[position.x + position.y * DrawBufferWidth] = 0x1;

    } else {
       // memory.mousePosition = Vec2f{-1.0f, -1.0f};
    }

	return GameOutput{
		.shouldQuit = input.closeRequested,
        .shouldHideMouse = input.mouse.trackLength ? eTRUE : eFALSE,
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
