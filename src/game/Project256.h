#ifndef PROJECT_256_H
#define PROJECT_256_H

#define constant static const

#ifndef _WIN32
#include "CoreFoundation/CFBase.h"
#else
#define CF_SWIFT_NAME(a)
#endif // !_WIN32

#include "Profiling/Timings.h"

// global static constants

constant long MemorySize = 640 * 1024; // 640kbytes

constant unsigned DrawBufferWidth = 320;
constant unsigned DrawBufferHeight = 200;


constant unsigned DrawAspectH = 4;
constant unsigned DrawAspectV = 3;

constant unsigned InputMouseMaxTrackLength = 32;

constant unsigned InputMaxControllers = 5;
constant unsigned InputMaxTaps = 20;
constant unsigned InputMaxTextLength = 256;

#ifdef __cplusplus
#include <cstdint>
#define _Bool bool
extern "C" {
#else
#include "stdint.h"
#endif

struct Vec2f {
	float x, y;
};

struct Vec2i {
	int x, y;
};

struct Button {
	int transitionCount;
    _Bool endedDown;
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
    _Bool endedOver;
	struct Vec2f relativeMovement;
	struct Vec2f scroll;
};


struct GameInput {
    double elapsedTime_s;
    long long upTime_microseconds;
    long long unsigned frameNumber;

    struct Controller controllers[InputMaxControllers];
    unsigned controllerCount;

    struct Tap taps[InputMaxTaps];
    unsigned tapCount;

    _Bool hasMouse;
    struct Mouse mouse;

    char text_utf8[InputMaxTextLength];
    unsigned textLength;

    _Bool closeRequested;
};

struct Rumble {
	float left, right;
};


struct GameOutput {
    _Bool shouldQuit;
    _Bool needTextInput;
    _Bool shouldPinMouse;
    _Bool shouldShowSystemCursor;
	struct Rumble rumble[InputMaxControllers];
};

struct Vec2f clipSpaceDrawBufferScale(unsigned int viewportWidth, unsigned int viewportHeight);

struct GameOutput doGameThings(struct GameInput* input, void* memory);
void writeDrawBuffer(void* memory, void* buffer);

#ifdef __cplusplus
}
#endif

#endif
