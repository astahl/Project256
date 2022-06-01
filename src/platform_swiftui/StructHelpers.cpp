//
//  StructHelpers.cpp
//  Project256
//
//  Created by Andreas Stahl on 01.06.22.
//

#define CXX
#include "StructHelpers.h"
#include "../game/Project256.h"


extern "C" {

void inputPushMouseTrack(GameInput* pInput, float x, float y)
{
    GameInput &input = *pInput;
    if (input.mouse.trackLength < MouseMaxTrackLength)
        input.mouse.track[input.mouse.trackLength++] = {x, y};
}

void inputPushUtf8Byte(GameInput* pInput, char c)
{
    GameInput &input = *pInput;
    if (input.textLength < InputMaxTextLength)
        input.text_utf8[input.textLength++] = c;
}

void inputClear(GameInput* pInput)
{
    *pInput = GameInput{};
}

}
