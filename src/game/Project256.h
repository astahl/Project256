/// TODOs
///
/// General
///
/// *
///
/// Windows
///
/// * Fix scaling blur issues (custom supersampling shader?)
///
/// Mac
///
/// * Fix Audio crackling issues
/// * Window new/ close / quit handling
///
/// Linux
///
/// * [] Decide on project structure / platform technologies
///  * Application framework? (GTK? Qt?)
///  * Graphics? (Cairo, Vulkan?)
///  * Gamepad input? (Steam?)
///  * Sound Output? (PulseAudio, Alsa?)
#ifndef PROJECT_256_H
#define PROJECT_256_H

#include "defines.h"

// global static constants

constant long MemorySize = 1024 * 1024 * 8; // 8 MBytes

constant unsigned AudioFramesPerBuffer = 1024;
constant unsigned AudioBufferCount = 5;
constant unsigned AudioFramesPerSecond = 48000;
constant unsigned AudioChannelsPerFrame = 2;
constant unsigned AudioBitsPerSample = 16;

constant unsigned DrawBufferWidth = 160;
constant unsigned DrawBufferHeight = 200;

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
    ControllerSubTypeNone = 0,
    ControllerSubTypeKeyboard = 1,
    ControllerSubTypeMouse = 2,
    ControllerSubTypeKeyboardAndMouse = 3,
    ControllerSubTypeXBox,
    ControllerSubTypeSteam,
    ControllerSubTypePlayStation,
    ControllerSubTypeWiiMote,
    ControllerSubTypeGeneric,
    ControllerSubTypeGenericSNES,
    ControllerSubTypeGenericNES,
    ControllerSubTypeGenericTwoButton,
    ControllerSubTypeGenericSingleButton
};


struct GameController {
    enum ControllerSubType subType;
    _Bool isConnected;
    // if any action is registered on buttons or axes
    _Bool isActive;
    struct Axis2 stickLeft, stickRight, dPad;
    struct Axis1 triggerLeft, triggerRight;
    struct Button shoulderLeft, shoulderRight, buttonBack, buttonStart, buttonX, buttonY, buttonA, buttonB, buttonStickLeft, buttonStickRight, buttonGripLeft, buttonGripRight;
} CF_SWIFT_NAME(P256GameController);

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
    // filename, buffer, buffersize -> actual read size
    long long (*readFile)(const char*, unsigned char*, long long);
    // filename, argb buffer, width, height -> success
    _Bool (*readImage)(const char*, unsigned int*, int, int);
};

struct AudioBufferDescriptor {
    unsigned long long timestamp;
    double sampleTime;
    double sampleRate;
    unsigned framesPerBuffer;
    unsigned channelsPerFrame;
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
void writeAudioBuffer(void* memory, void* buffer, struct AudioBufferDescriptor bufferDescriptor);

#ifdef __cplusplus
}
#endif

#endif
