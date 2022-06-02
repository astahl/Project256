#define CXX
#include "Project256.h"
#include <cstdint>

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

GameOutput doGameThings(GameInput* input, void* memory)
{
    if (memory == nullptr)
    {
        return GameOutput {

        };
    }
	return GameOutput{
		.shouldQuit = input->closeRequested
	};
}

void writeDrawBuffer(void* memory, void* buffer)
{
    if (memory == nullptr) {
        uint32_t* pixel = reinterpret_cast<uint32_t*>(buffer);
        for (unsigned y = 0; y < DrawBufferHeight; ++y)
        for (unsigned x = 0; x < DrawBufferWidth; ++x)
        {
            *pixel++ = 0xFF000000 | ((y % 2) && (x % 2) ? 0xFFFFFF : 0x000000) ; // argb
        }
    }
}

}
