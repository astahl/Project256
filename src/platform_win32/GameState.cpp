#include "GameState.h"
#include "Xinput.h"
#include <string>


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
    keyEventCount++;
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
        gamepadButton(axis.left, false);
    }
    else if (valueX < -Deadzone) {
        axis.end.x = static_cast<float>(valueX + Deadzone) * deadFactorNegative;
        gamepadButton(axis.left, true);
        gamepadButton(axis.right, false);
    }
    else {
        gamepadButton(axis.left, false);
        gamepadButton(axis.right, false);
        changeX = false;
    }

    if (valueY > Deadzone) {
        axis.end.y = static_cast<float>(valueY - Deadzone) * deadFactorPositive;
        gamepadButton(axis.up, true);
        gamepadButton(axis.down, false);
    }
    else if (valueY < -Deadzone) {
        axis.end.y = static_cast<float>(valueY + Deadzone) * deadFactorNegative;
        gamepadButton(axis.down, true);
        gamepadButton(axis.up, false);
    }
    else {
        gamepadButton(axis.up, false);
        gamepadButton(axis.down, false);
        changeY = false;
    }
    return changeX || changeY;
}

void PlatformInput::updateGameInput(GameInput& gameInput) {
    auto elapsed = frameTime.elapsed();
    upTime += elapsed.microseconds;
    gameInput.elapsedTime_s = elapsed.seconds;
    gameInput.upTime_microseconds = upTime;
    gameInput.frameNumber = frameCount++;

    GameController& kbm = gameInput.controllers[0];
    kbm.isConnected = true;
    for (int i = 0; i < keyEventCount; ++i) {
        auto& keyEvent = keyEvents[i];
        bool down = !keyEvent.isUp;
        bool isAction = false;
        switch (keyEvent.virtualKeyCode) {
        case 'W': isAction = gamepadButton(kbm.stickLeft.up, down); break;
        case 'A': isAction = gamepadButton(kbm.stickLeft.left, down); break;
        case 'S': isAction = gamepadButton(kbm.stickLeft.down, down); break;
        case 'D': isAction = gamepadButton(kbm.stickLeft.right, down); break;
        case '1': isAction = gamepadButton(kbm.dPad.up, down); break;
        case '2': isAction = gamepadButton(kbm.dPad.left, down); break;
        case '4': isAction = gamepadButton(kbm.dPad.down, down); break;
        case '3': isAction = gamepadButton(kbm.dPad.right, down); break;
        case VK_UP: isAction = gamepadButton(kbm.stickRight.up, down); break;
        case VK_LEFT: isAction = gamepadButton(kbm.stickRight.left, down); break;
        case VK_DOWN: isAction = gamepadButton(kbm.stickRight.down, down); break;
        case VK_RIGHT: isAction = gamepadButton(kbm.stickRight.right, down); break;

        case VK_CONTROL: isAction = gamepadButton(kbm.shoulderLeft, down); break;
        case VK_SHIFT: isAction = gamepadButton(kbm.shoulderRight, down); break;
        case VK_SPACE: isAction = gamepadButton(kbm.buttonA, down); break;
        case 'F': isAction = gamepadButton(kbm.buttonB, down); break;
        case 'R': isAction = gamepadButton(kbm.buttonX, down); break;
        case 'C': isAction = gamepadButton(kbm.buttonY, down); break;
        case VK_TAB: isAction = gamepadButton(kbm.buttonStickRight, down); break;
        case VK_ESCAPE: isAction = gamepadButton(kbm.buttonBack, down); break;
        case VK_RETURN: isAction = gamepadButton(kbm.buttonStart, down); break;
        default: isAction = false;
        }
        kbm.isActive |= isAction;
    }
    keyEventCount = 0;

    kbm.stickRight.end = gameInput.mouse.relativeMovement;
    kbm.triggerLeft.trigger = gameInput.mouse.buttonLeft;
    kbm.triggerRight.trigger = gameInput.mouse.buttonRight;
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


INT64 readFileDEBUG(const char* filename, unsigned char* buffer, INT64 bufferSize) {

    int requiredWideLength = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, filename, -1, NULL, 0);
    std::wstring wideString(requiredWideLength, L'\0');
    MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, filename, -1, wideString.data(), static_cast<int>(wideString.size()));

    std::wstring pathBuf(512, L'\0');
    GetModuleFileName(nullptr, pathBuf.data(), 512);
    size_t lastSlashPos = pathBuf.find_last_of(L'\\');
    if (lastSlashPos != std::wstring::npos) pathBuf.erase(lastSlashPos + 1);
    std::wstring filePath = pathBuf + wideString;
    HANDLE file = CreateFile2(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        exit(GetLastError());
    }
    DWORD read{};
    if (ReadFile(file, buffer, static_cast<DWORD>(bufferSize), &read, NULL)) {
        return read;
    }
    exit(GetLastError());
}

GameOutput GameState::tick() {
    profiling_time_interval(&GameState::timingData, eTimerTickToTick, eTimingTickToTick);
    profiling_time_set(&GameState::timingData, eTimerTickToTick);

    profiling_time_set(&GameState::timingData, eTimerTick);

   
    platform.updateGameInput(input);
    platform.pollXInput(input);
    GameOutput output{};

    profiling_time_interval(&GameState::timingData, eTimerTick, eTimingTickSetup);
    output = doGameThings(&input, memory, {
        .readFile = readFileDEBUG
        });
    profiling_time_interval(&GameState::timingData, eTimerTick, eTimingTickDo);

    cleanInput(&input);
    
    return output;
}