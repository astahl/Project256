#pragma once


const unsigned DrawBufferWidth = 320;
const unsigned DrawBufferHeight = 240;

extern "C" {


struct Vec2f {
	float x, y;
};

struct Vec2i {
	int x, y;
};

struct Button {
	int transistionCount;
	bool endedDown;
};

struct Axis2
{
	Vec2f value;
	Button up, down, left, right;
};

struct Controller
{
	Axis2 left, right, dPad;
};

struct Tap
{
	Vec2i position;
	float force;
};

struct Mouse
{
	const static unsigned MAX_TRACK_LENGTH = 16;
	Button buttonLeft, buttonRight, buttonMiddle;
	Vec2i track[MAX_TRACK_LENGTH];
	Vec2f relativeMovement;
	Axis2 scroll;
};

struct Rumble
{
	float left, right;
};

struct GameInput
{
	const static unsigned MAX_CONTROLLERS = 5;
	const static unsigned MAX_TAPS = 20;
	const static unsigned MAX_TEXT_LENGTH = 1024;

	double elapsedTime_s;
	long long unsigned upTime_µs;
	long long unsigned frameNumber;

	Controller controllers[MAX_CONTROLLERS];
	unsigned controllerCount;

	Tap taps[MAX_TAPS];
	unsigned tapCount;

	bool hasMouse;
	Mouse mouse;

	char text_utf8[MAX_TEXT_LENGTH];
	unsigned textLength;

	bool closeRequested;
};

struct GameOutput {
	bool shouldQuit;
	bool needTextInput;
	Rumble rumble[GameInput::MAX_CONTROLLERS];
};


GameOutput doGameThings(GameInput* input, void* memory);
void writeDrawBuffer(void* memory, void* buffer);

}