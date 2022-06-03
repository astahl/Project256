#pragma once

const long MemorySize = 32 * 1024; // 32k

const unsigned DrawBufferWidth = 320;
const unsigned DrawBufferHeight = 200;
const unsigned DrawAspectH = 4;
const unsigned DrawAspectV = 3;

const unsigned MouseMaxTrackLength = 32;

const unsigned InputMaxControllers = 5;
const unsigned InputMaxTaps = 20;
const unsigned InputMaxTextLength = 256;

#ifdef CXX
extern "C" {
#define ___BOOLEAN bool
#endif

#ifndef ___BOOLEAN
#define ___BOOLEAN int
#endif


struct Vec2f {
	float x, y;
};

struct Vec2i {
	int x, y;
};

struct Button {
	int transistionCount;
	___BOOLEAN endedDown;
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
	struct Vec2f track[MouseMaxTrackLength];
    unsigned trackLength;
	struct Vec2f relativeMovement;
	struct Axis2 scroll;
};

struct Rumble {
	float left, right;
};

struct GameInput {
	double elapsedTime_s;
	long long unsigned upTime_microseconds;
	long long unsigned frameNumber;

	struct Controller controllers[InputMaxControllers];
	unsigned controllerCount;

	struct Tap taps[InputMaxTaps];
	unsigned tapCount;

	___BOOLEAN hasMouse;
	struct Mouse mouse;

	char text_utf8[InputMaxTextLength];
	unsigned textLength;

	___BOOLEAN closeRequested;
};

struct GameOutput {
	___BOOLEAN shouldQuit;
	___BOOLEAN needTextInput;
	struct Rumble rumble[InputMaxControllers];
};

struct Vec2f clipSpaceDrawBufferScale(unsigned int viewportWidth, unsigned int viewportHeight);

struct GameOutput doGameThings(struct GameInput* input, void* memory);
void writeDrawBuffer(void* memory, void* buffer);

#ifdef CXX
}
#endif
