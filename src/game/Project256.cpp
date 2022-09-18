#include "Project256.h"
#include <cassert>
#include "TestBed.hpp"
#include "Minesweeper.hpp"


using Game = TestBed;
//using Game = Minesweeper;

extern "C" {

Vec2f clipSpaceDrawBufferScale(unsigned int viewportWidth, unsigned int viewportHeight)
{
    // we COULD assert that viewport dimensions are not greater than UINT16_MAX each...
    uint32_t viewportAspectRatio16_16 = (viewportWidth << 16) / viewportHeight;
    uint32_t drawAspectRatio16_16 = (DrawAspectH << 16) / DrawAspectV;

    uint32_t widthOnTarget = viewportWidth;
    uint32_t heightOnTarget = viewportHeight;

    if (viewportAspectRatio16_16 > drawAspectRatio16_16) {
        // window is wider than the draw target, shrink on x
        widthOnTarget = (heightOnTarget * drawAspectRatio16_16) >> 16;
    }
    else if (drawAspectRatio16_16 > viewportAspectRatio16_16) {
        heightOnTarget = (widthOnTarget << 16) / drawAspectRatio16_16;
    }

    return {
        .x = static_cast<float>(widthOnTarget) / viewportWidth,
        .y = static_cast<float>(heightOnTarget) / viewportHeight,
    };
}


void cleanInput(GameInput* input) {
    for (int i = 0; i < InputMaxControllers; ++i) {
        auto& controller = input->controllers[i];
        Axis1* axis = &controller.triggerLeft;
        for (int axis1Index = 0; axis1Index < InputControllerAxis1Count; ++axis1Index)
        {
            axis[axis1Index].trigger.transitionCount = 0;
            axis[axis1Index].start = axis[axis1Index].end;
        }
        Axis2* axis2 = &controller.stickLeft;
        for (int axis2Index = 0; axis2Index < InputControllerAxis1Count; ++axis2Index)
        {
            axis2[axis2Index].left.transitionCount = 0;
            axis2[axis2Index].up.transitionCount = 0;
            axis2[axis2Index].down.transitionCount = 0;
            axis2[axis2Index].right.transitionCount = 0;
            axis2[axis2Index].start = axis2[axis2Index].end;
            if (!axis2[axis2Index].latches)
                axis2[axis2Index].end = Vec2f{};
        }
        Button* button = &controller.shoulderLeft;
        for (int buttonIndex = 0; buttonIndex < InputControllerButtonCount; ++buttonIndex)
        {
            button[buttonIndex].transitionCount = 0;
        }
    }
    auto& mouse = input->mouse;
    mouse.buttonLeft.transitionCount = 0;
    mouse.buttonRight.transitionCount = 0;
    mouse.buttonMiddle.transitionCount = 0;
    if (mouse.trackLength && mouse.endedOver) {
        mouse.track[0] = mouse.track[mouse.trackLength - 1];
        mouse.trackLength = 1;
    } else {
        mouse.trackLength = 0;
    }
    mouse.relativeMovement.x = 0;
    mouse.relativeMovement.y = 0;


    input->textLength = 0;
    input->closeRequested = false;
    input->tapCount = 0;
}


GameOutput doGameThings(GameInput* pInput, void* pMemory, PlatformCallbacks platform)
{
    assert(pInput != nullptr);
    assert(pMemory != nullptr);
    assert(platform.readFile != nullptr);
    assert(platform.readImage != nullptr);

    static_assert(sizeof(Game::MemoryLayout) <= MemorySize, "MemorySize is too small to hold GameMemory struct");

    auto& memory = *reinterpret_cast<Game::MemoryLayout*>(pMemory);
    GameInput& input = *pInput;

    return Game::doGameThings(memory, input, platform);
}

void writeDrawBuffer(void* pMemory, void* buffer)
{
    assert(buffer != nullptr);
    assert(pMemory != nullptr);

    auto& memory = *reinterpret_cast<Game::MemoryLayout*>(pMemory);
    auto& drawBuffer = *reinterpret_cast<Game::DrawBuffer*>(buffer);
    Game::writeDrawBuffer(memory, drawBuffer);
}


void writeAudioBuffer(void* pMemory, void* buffer, struct AudioBufferDescriptor bufferDescriptor)
{
    assert(buffer != nullptr);
    assert(pMemory != nullptr);

    auto& memory = *reinterpret_cast<Game::MemoryLayout*>(pMemory);
    auto& audioBuffer = *reinterpret_cast<Game::AudioBuffer*>(buffer);
    Game::writeAudioBuffer(memory, audioBuffer, bufferDescriptor);
    
}

}
