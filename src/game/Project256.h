#ifndef PROJECT_256_H
#define PROJECT_256_H

const long MemorySize = 640 * 1024; // 640kbytes

const unsigned DrawBufferWidth = 160;
const unsigned DrawBufferHeight = 200;
const unsigned DrawBufferBpp = 32;
const unsigned DrawBufferBytes = DrawBufferWidth * DrawBufferHeight * DrawBufferBpp / 8;

const unsigned DrawAspectH = 4;
const unsigned DrawAspectV = 3;

const unsigned MouseMaxTrackLength = 32;

const unsigned InputMaxControllers = 5;
const unsigned InputMaxTaps = 20;
const unsigned InputMaxTextLength = 256;

#ifdef CXX
extern "C" {
#endif

enum BOOLEAN_ENUM {
    eFALSE,
    eTRUE
};

struct Vec2f {
	float x, y;
};

struct Vec2i {
	int x, y;
};

struct Button {
	int transistionCount;
	enum BOOLEAN_ENUM endedDown;
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

    enum BOOLEAN_ENUM hasMouse;
	struct Mouse mouse;

	char text_utf8[InputMaxTextLength];
	unsigned textLength;

	enum BOOLEAN_ENUM closeRequested;
};

struct GameOutput {
	enum BOOLEAN_ENUM shouldQuit;
	enum BOOLEAN_ENUM needTextInput;
    enum BOOLEAN_ENUM shouldPinMouse;
    enum BOOLEAN_ENUM shouldHideMouse;
	struct Rumble rumble[InputMaxControllers];
};

struct Vec2f clipSpaceDrawBufferScale(unsigned int viewportWidth, unsigned int viewportHeight);

struct GameOutput doGameThings(struct GameInput* input, void* memory);
void writeDrawBuffer(void* memory, void* buffer);

#ifdef CXX
}
#endif

#endif
