#ifndef PROJECT_256_H
#define PROJECT_256_H

const long MemorySize = 640 * 1024; // 640kbytes

const unsigned DrawBufferWidth = 160;
const unsigned DrawBufferHeight = 200;

const unsigned DrawAspectH = 4;
const unsigned DrawAspectV = 3;

const unsigned InputMouseMaxTrackLength = 32;

const unsigned InputMaxControllers = 5;
const unsigned InputMaxTaps = 20;
const unsigned InputMaxTextLength = 256;

#ifdef CXX
#include <cstdint>

extern "C" {
#endif

enum boole {
    eFALSE = 0,
#ifdef __UINT32_MAX__
    eTRUE = __UINT32_MAX__ 
#else
	eTRUE = UINT32_MAX
#endif
};

#define BOOLEAN_ENUM enum boole

struct Vec2f {
	float x, y;
};

struct Vec2i {
	int x, y;
};

struct Button {
	int transistionCount;
    BOOLEAN_ENUM endedDown;
};

struct Axis2 {
	struct Vec2f value;
	struct Button up, down, left, right;
};

struct Controller {
	struct Axis2 left, right, dPad;
};

struct Tap {
	struct Vec2i position;
	float force;
};

struct Mouse {
	struct Button buttonLeft, buttonRight, buttonMiddle;
	struct Vec2f track[InputMouseMaxTrackLength];
    unsigned trackLength;
    BOOLEAN_ENUM endedOver;
	struct Vec2f relativeMovement;
	struct Axis2 scroll;
};

struct Rumble {
	float left, right;
};

struct GameInput {
	double elapsedTime_s;
	long long upTime_microseconds;
	long long unsigned frameNumber;

	struct Controller controllers[InputMaxControllers];
	unsigned controllerCount;

	struct Tap taps[InputMaxTaps];
	unsigned tapCount;

    BOOLEAN_ENUM hasMouse;
	struct Mouse mouse;

	char text_utf8[InputMaxTextLength];
	unsigned textLength;

    BOOLEAN_ENUM closeRequested;
};

struct GameOutput {
    BOOLEAN_ENUM shouldQuit;
    BOOLEAN_ENUM needTextInput;
    BOOLEAN_ENUM shouldPinMouse;
    BOOLEAN_ENUM shouldHideMouse;
	struct Rumble rumble[InputMaxControllers];
};

struct Vec2f clipSpaceDrawBufferScale(unsigned int viewportWidth, unsigned int viewportHeight);

struct GameOutput doGameThings(struct GameInput* input, void* memory);
void writeDrawBuffer(void* memory, void* buffer);

#ifdef CXX
}
#endif

#endif
