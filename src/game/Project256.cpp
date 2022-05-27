#define CXX
#include "Project256.h"
#include <cstdint>

extern "C" {

GameOutput doGameThings(GameInput* input, void* memory)
{
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
