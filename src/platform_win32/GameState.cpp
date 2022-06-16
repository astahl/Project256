#include "GameState.h"
#include "Xinput.h"

TimingData GameState::timingData{
    .getPlatformTimeMicroseconds = []() {
        LARGE_INTEGER frequency, ticks;
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&ticks);
        return (ticks.QuadPart * 1'000'000) / frequency.QuadPart;
    }
};

void PlatformInput::pushKeyEvent(WindowsKeyEvent keyEvent)
{
    constant byte SC_LCtrl = 0x1D;
    constant byte SC_RAlt = 0x38;
    constant bool CheckForAltGr = true;
    int insertPosition = this->keyEventCount;
    if constexpr (CheckForAltGr) {
   
        // AltGr is chord of LCtrl + RAlt, which is less than useful for keyboard as controller
        // We need to look into buffer if previous key is lctrl in the same direction and overwrite it.

        if (insertPosition > 0 && keyEvent.isExtended && keyEvent.scanCode == SC_RAlt) {
            WindowsKeyEvent& previous = this->keyEvents[insertPosition - 1];
            if (previous.scanCode == SC_LCtrl && keyEvent.isUp == previous.isUp) {
                insertPosition -= 1;
            }
        }
    }

    this->keyEvents[insertPosition] = keyEvent;
}

void PlatformInput::updateGameInput(GameInput& gameInput) {
    auto elapsed = frameTime.elapsed();
    upTime += elapsed.microseconds;
    gameInput.elapsedTime_s = elapsed.seconds;
    gameInput.upTime_microseconds = upTime;
    gameInput.frameNumber = frameCount++;
}


inline internalfunc bool gamepadButton(Button& button, bool isDown) {
    bool wasDown = button.endedDown;
    button.transitionCount += isDown != wasDown ? 1 : 0;
    button.endedDown = isDown;
    return isDown || wasDown;
}


inline internalfunc bool gamepadButton(Button& button, WORD buttonFlags, WORD mask) {
    return gamepadButton(button, (buttonFlags & mask) == mask);
}


inline internalfunc bool gamepadAxis8bit(Axis1& axis, BYTE value) {
    axis.isAnalog = true;
    axis.end = static_cast<float>(value) / 255.0f;
    gamepadButton(axis.trigger, value != 0);
    return value != 0;
}

template<SHORT Deadzone>
inline internalfunc bool gamepadAxis16bit(Axis2& axis, SHORT valueX, SHORT valueY) {
    compiletime float deadFactorPositive =  1.0f / (32767 - Deadzone);
    compiletime float deadFactorNegative = 1.0f / (32768 - Deadzone);
    axis.isAnalog = true;
    bool changeX = true;
    bool changeY = true;
    if (valueX > Deadzone) {
        axis.end.x = static_cast<float>(valueX - Deadzone) * deadFactorPositive;
        gamepadButton(axis.right, true);
    }
    else if (valueX < -Deadzone) {
        axis.end.x = static_cast<float>(valueX + Deadzone) * deadFactorNegative;
        gamepadButton(axis.left, true);
    }
    else {
        changeX = false;
    }

    if (valueY > Deadzone) {
        axis.end.y = static_cast<float>(valueY - Deadzone) * deadFactorPositive;
        gamepadButton(axis.up, true);
    }
    else if (valueY < -Deadzone) {
        axis.end.y = static_cast<float>(valueY + Deadzone) * deadFactorNegative;
        gamepadButton(axis.down, true);
    }
    else {
        changeY = false;
    }
    return changeX || changeY;
}



void PlatformInput::pollXInput(GameInput& gameInput)
{
    constant BYTE TriggerThreshold = 1;
    static_assert(XUSER_MAX_COUNT + 1 <= InputMaxControllers, "Someone updated XInput to support more controllers! Check temperature in Hell.");
    for (DWORD userIndex = 0; userIndex < XUSER_MAX_COUNT; ++userIndex) {
        GameController& controller = gameInput.controllers[1 + userIndex];
        XINPUT_STATE state{};
        DWORD error = XInputGetState(userIndex, &state);
        if (error == ERROR_DEVICE_NOT_CONNECTED) {
            controller = GameController{};
            continue;
        }
        else if (error != ERROR_SUCCESS)
        {
            // different kind of error
            DebugBreak();
            controller = GameController{};
            continue;
        }
        controller.subType = ControllerSubTypeXBox;
        XINPUT_GAMEPAD& gamepad = state.Gamepad;
        controller.isConnected = true;
        controller.isActive |= gamepadButton(controller.buttonA, gamepad.wButtons, XINPUT_GAMEPAD_A);
        controller.isActive |= gamepadButton(controller.buttonB, gamepad.wButtons, XINPUT_GAMEPAD_B);
        controller.isActive |= gamepadButton(controller.buttonX, gamepad.wButtons, XINPUT_GAMEPAD_X);
        controller.isActive |= gamepadButton(controller.buttonY, gamepad.wButtons, XINPUT_GAMEPAD_Y);
        controller.isActive |= gamepadButton(controller.buttonStart, gamepad.wButtons, XINPUT_GAMEPAD_START);
        controller.isActive |= gamepadButton(controller.buttonBack, gamepad.wButtons, XINPUT_GAMEPAD_BACK);
        controller.isActive |= gamepadButton(controller.buttonStickLeft, gamepad.wButtons, XINPUT_GAMEPAD_LEFT_THUMB);
        controller.isActive |= gamepadButton(controller.buttonStickRight, gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB);
        controller.isActive |= gamepadButton(controller.shoulderLeft, gamepad.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER);
        controller.isActive |= gamepadButton(controller.shoulderRight, gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER);
        controller.isActive |= gamepadButton(controller.dPad.up, gamepad.wButtons, XINPUT_GAMEPAD_DPAD_UP);
        controller.isActive |= gamepadButton(controller.dPad.down, gamepad.wButtons, XINPUT_GAMEPAD_DPAD_DOWN);
        controller.isActive |= gamepadButton(controller.dPad.left, gamepad.wButtons, XINPUT_GAMEPAD_DPAD_LEFT);
        controller.isActive |= gamepadButton(controller.dPad.right, gamepad.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT);
    
        controller.isActive |= gamepadAxis8bit(controller.triggerLeft, gamepad.bLeftTrigger);
        controller.isActive |= gamepadAxis8bit(controller.triggerRight, gamepad.bRightTrigger);
        
        controller.isActive |= gamepadAxis16bit<XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE>(controller.stickLeft, 
            gamepad.sThumbLX, gamepad.sThumbLY);
        controller.isActive |= gamepadAxis16bit<XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE>(controller.stickRight, 
            gamepad.sThumbRX, gamepad.sThumbRY);
    }
}


GameState::GameState() {
    this->memory = reinterpret_cast<byte*>(VirtualAlloc(LPVOID(2LL * 1024 * 1024 * 1024), MemorySize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    this->drawBuffer = reinterpret_cast<byte*>(VirtualAlloc(LPVOID(2LL * 1024 * 1024 * 1024 + MemorySize), 4 * DrawBufferHeight * DrawBufferWidth, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
}

GameOutput GameState::tick() {
    profiling_time_interval(&GameState::timingData, eTimerTickToTick, eTimingTickToTick);
    profiling_time_set(&GameState::timingData, eTimerTickToTick);

    profiling_time_set(&GameState::timingData, eTimerTick);

   
    platform.updateGameInput(input);
    platform.pollXInput(input);
    GameOutput output{};

    profiling_time_interval(&GameState::timingData, eTimerTick, eTimingTickSetup);
    output = doGameThings(&input, memory);
    profiling_time_interval(&GameState::timingData, eTimerTick, eTimingTickDo);

    cleanInput(&input);
    
    return output;
}