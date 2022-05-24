#include "Project256.h"


extern "C" {

GameOutput doGameThings(GameInput* input, void* memory)
{
	return GameOutput{
		.shouldQuit = input->closeRequested
	};
}

void writeDrawBuffer(void* memory, void* buffer)
{
	unsigned char* pixel = reinterpret_cast<unsigned char*>(buffer);
	for (unsigned y = 0; y < DrawBufferHeight; ++y)
	for (unsigned x = 0; x < DrawBufferWidth; ++x)
	{
		*pixel++ = 0xFF;
		*pixel++ = 0xCC;
		*pixel++ = 0x00;
		*pixel++ = 0xFF;
	}
		
}

}