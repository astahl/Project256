#ifndef PROJECT_256_H
#define PROJECT_256_H

#include "defines.h"

#include "Profiling/Timings.h"

// global static constants

constant long MemorySize = 640 * 1024; // 640kbytes

constant unsigned DrawBufferWidth = 640;
constant unsigned DrawBufferHeight = 350;

constant unsigned DrawAspectH = 4;
constant unsigned DrawAspectV = 3;

constant unsigned InputMouseMaxTrackLength = 32;

constant unsigned InputMaxControllers = 5;
constant unsigned InputControllerAxis1Count = 2;
constant unsigned InputControllerAxis2Count = 3;
constant unsigned InputControllerButtonCount = 12;

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

struct Recti {
    struct Vec2i origin, size;
};

struct Button {
	int transitionCount;
    _Bool endedDown;
} CF_SWIFT_NAME(GameButton);

struct Axis1 {
    _Bool isAnalog;
    float start, end;
    struct Button trigger;
};

struct Axis2 {
    _Bool isAnalog;
    struct Vec2f start, end;
    struct Button up, down, left, right;
    _Bool latches;
};


enum ControllerSubType {
    ControllerSubTypeNone,
    ControllerSubTypeKeyboard,
    ControllerSubTypeMouse,
    ControllerSubTypeKeyboardAndMouse,
    ControllerSubTypeXBox,
    ControllerSubTypeSteam,
    ControllerSubTypePlayStation,
    ControllerSubTypeWiiMote,
    ControllerSubTypeGenericSNES,
    ControllerSubTypeGenericNES,
    ControllerSubTypeGenericTwoButton,
    ControllerSubTypeGenericSingleButton
};

#pragma warning (push)
#pragma warning (disable : 4201)

struct GameController {
    enum ControllerSubType subType;
    _Bool isConnected;
    // if any action is registered on buttons or axes
    _Bool isActive;
    union {
        struct {
            struct Axis2 stickLeft, stickRight, dPad;
        };
        struct Axis2 axes2[InputControllerAxis2Count];
    };
    union {
        struct {
            struct Axis1 triggerLeft, triggerRight;
        };
        struct Axis1 axes1[InputControllerAxis1Count];
    };
    union {
        struct {
            struct Button shoulderLeft, shoulderRight, buttonBack, buttonStart, buttonX, buttonY, buttonA, buttonB, buttonStickLeft, buttonStickRight, buttonGripLeft, buttonGripRight;
        };
        struct Button buttons[InputControllerButtonCount];
    };
};

#pragma warning (pop)

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

    struct GameController controllers[InputMaxControllers];
    unsigned controllerCount;

    struct Tap taps[InputMaxTaps];
    unsigned tapCount;

    struct Mouse mouse;

    char text_utf8[InputMaxTextLength];
    unsigned textLength;

    _Bool closeRequested;
};

struct PlatformCallbacks {
    long long (*readFile)(const char*, unsigned char*, long long);
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
void cleanInput(struct GameInput* input);
struct GameOutput doGameThings(struct GameInput* input, void* memory, struct PlatformCallbacks callbacks);
void writeDrawBuffer(void* memory, void* buffer);

#ifdef __cplusplus
}
#endif

#endif
