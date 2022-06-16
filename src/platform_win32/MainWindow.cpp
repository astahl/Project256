#include "MainWindow.h"
#include "Direct3D12View.h"
#include <initializer_list>
#include "../game/Project256.h"

#include "GameState.h"

enum class Timers : UINT_PTR {
    HighFrequency,
    LowFrequency,
};

static void setCursorVisible(bool shouldShow) {
    CURSORINFO cursorInfo{ .cbSize = sizeof(CURSORINFO) };
    GetCursorInfo(&cursorInfo);
    if (!shouldShow && cursorInfo.flags == CURSOR_SHOWING) // cursor is visible
    {
        ShowCursor(FALSE);
    }
    else if (shouldShow && cursorInfo.flags == 0)
    {
        ShowCursor(TRUE);
    }
}


MainWindow::MainWindow(HWND hwnd)
    : state(new GameState())
    , hwnd(hwnd)
{
    RECT rect{};
    GetWindowRect(hwnd, &rect);
    this->view = new Direct3D12View(hwnd, rect.right - rect.left, rect.bottom - rect.top);
    SetTimer(hwnd, static_cast<UINT_PTR>(Timers::HighFrequency), 1, NULL);
    SetTimer(hwnd, static_cast<UINT_PTR>(Timers::LowFrequency), 1000, NULL);
}

MainWindow::~MainWindow()
{
    KillTimer(hwnd, static_cast<UINT_PTR>(Timers::HighFrequency));
}

void MainWindow::onTick() {
    const auto output = state->tick();
    if (output.shouldQuit) {
        OutputDebugStringA("Should Quit");
        DestroyWindow(hwnd);
    }

    if ((output.shouldShowSystemCursor) != state->forceCursor) {
        state->forceCursor = output.shouldShowSystemCursor;
        setCursorVisible(state->forceCursor);
    }

    if (output.shouldPinMouse) {
        RECT rect{};
        GetClientRect(hwnd, &rect);
        POINT center{ .x = rect.left + (rect.right - rect.left) / 2, .y = rect.top + (rect.bottom - rect.top) / 2 };
        ClientToScreen(hwnd, &center);
        SetCursorPos(center.x, center.y);
    }

    InvalidateRect(hwnd, NULL, FALSE);

    profiling_time_interval(&GameState::timingData, eTimerTick, eTimingTickPost);
}

void MainWindow::onPaint() {
    profiling_time_set(&GameState::timingData, eTimerDraw);
    profiling_time_set(&GameState::timingData, eTimerBufferCopy);
    writeDrawBuffer(state->memory, state->drawBuffer);
    profiling_time_interval(&GameState::timingData, eTimerBufferCopy, eTimingBufferCopy);
    this->view->SetDrawBuffer(state->drawBuffer);
    profiling_time_interval(&GameState::timingData, eTimerDraw, eTimingDrawBefore);
    this->view->Draw();
    ValidateRect(hwnd, NULL);
}

void MainWindow::onResize() {
    RECT rect{};
    GetWindowRect(hwnd, &rect);
    
    this->view->Resize(rect.right - rect.left, rect.bottom - rect.top);
}

void MainWindow::onClose() {
    this->state->input.closeRequested = true;
}

void MainWindow::onTimer(WPARAM timerId) {
    switch (static_cast<Timers>(timerId)) {
    case Timers::HighFrequency:
        this->onTick();
        break;
    case Timers::LowFrequency:
        profiling_time_print(&GameState::timingData, this->profilingStringBuffer, PROFILING_STR_BUFFER_LENGTH);
        profiling_time_clear(&GameState::timingData);
        OutputDebugStringA(this->profilingStringBuffer);
        break;
    }
}

void MainWindow::onMouseMove(POINTS points) {
    auto& mouse = this->state->input.mouse;
    auto scale = this->view->currentScale();
    RECT windowRect{};
    GetClientRect(hwnd, &windowRect);
    const int width = windowRect.right - windowRect.left;
    const int height = windowRect.bottom - windowRect.top;
    auto normalizedToWindow = Vec2f{ .x = static_cast<float>(points.x) / width, .y = static_cast<float>(points.y) / height };
    auto relativeToCenter = Vec2f{ .x = (normalizedToWindow.x - 0.5f) * 2, .y = (normalizedToWindow.y  - 0.5f) * 2 };
    auto scaledPos = Vec2f{ .x = relativeToCenter.x / scale.x * 0.5f + 0.5f, .y = relativeToCenter.y / scale.y * -0.5f  + 0.5f };
    if (scaledPos.x < 0.0f || scaledPos.x >= 1.0f || scaledPos.y < 0.0f || scaledPos.y >= 1.0f) {
        // outside
        setCursorVisible(TRUE);
        mouse.endedOver = false;
    } else {
        setCursorVisible(state->forceCursor);
        auto pixelPos = Vec2f{ .x = scaledPos.x * DrawBufferWidth, .y = scaledPos.y * DrawBufferHeight };
        mouse.track[mouse.trackLength++] = pixelPos;
        mouse.endedOver = true;
    }
    mouse.relativeMovement.x += static_cast<float>(points.x - this->state->platform.lastCursorPosition.x) / scale.x;
    mouse.relativeMovement.y -= static_cast<float>(points.y - this->state->platform.lastCursorPosition.y) / scale.y;
    this->state->platform.lastCursorPosition = Vec2i{ .x = points.x, .y = points.y };
}

void MainWindow::onMouseLeave() {
    // nothing to do, really?
}

void MainWindow::onMouseButton(MouseButtons button, MouseButtonClick click) {
    auto& mouse = this->state->input.mouse;
    auto& btn = button == MouseButtons::Left ? mouse.buttonLeft : (button == MouseButtons::Right ? mouse.buttonRight : mouse.buttonMiddle);
    switch (click) {
    case MouseButtonClick::Down:
        btn.transitionCount++;
        btn.endedDown = true;
        break;
    case MouseButtonClick::Up:
        btn.transitionCount++;
        btn.endedDown = false;
        break;
    case MouseButtonClick::DoubleClick:
        break;
    }
}

void MainWindow::onKey(WORD virtualKeyCode, WORD keyFlags, WORD repeatCount)
{
    WindowsKeyEvent keyEvent {
        .virtualKeyCode = virtualKeyCode,
        .repeatCount = repeatCount,
        .scanCode = LOBYTE(keyFlags),
        .isExtended = (keyFlags & KF_EXTENDED) != 0,
        .isDialogMode = (keyFlags & KF_DLGMODE) != 0,
        .isMenuMode = (keyFlags & KF_MENUMODE) != 0,
        .isAltDown = (keyFlags & KF_ALTDOWN) != 0,
        .isRepeat = (keyFlags & KF_REPEAT) != 0,
        .isUp = (keyFlags & KF_UP) != 0,
    };
    this->state->platform.pushKeyEvent(keyEvent);
}

int MainWindow::doMainLoop() {
    MSG msg = { };

    BOOL bRet;

    while ((bRet = GetMessage(&msg, hwnd, 0, 0)) != 0)
    {
        if (bRet == -1)
        {
            // handle the error and possibly exit
            return GetLastError();
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static MainWindow* window = nullptr;

    switch (uMsg) {
    case WM_CREATE: {
        CREATESTRUCT* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
        MainWindow** windowPP = reinterpret_cast<MainWindow**>(createStruct->lpCreateParams);
        window = new MainWindow(hwnd);
        *windowPP = window;
    } break;
    case WM_SIZE: {
        window->onResize();
    } break;
    case WM_PAINT:
        window->onPaint();
        break;
    case WM_CLOSE:
        OutputDebugStringA("Close");
        window->onClose();
        return 0; 
    case WM_DESTROY:
        OutputDebugStringA("Destroy");
        PostQuitMessage(0);
        return 0;
    case WM_TIMER:
        window->onTimer(wParam);
        break;
    case WM_MOUSEMOVE:
        window->onMouseMove(MAKEPOINTS(lParam));
        break;
    case WM_LBUTTONDOWN: 
        window->onMouseButton(MouseButtons::Left, MouseButtonClick::Down);
        break;
    case WM_LBUTTONUP:
        window->onMouseButton(MouseButtons::Left, MouseButtonClick::Up);
        break;
    case WM_LBUTTONDBLCLK:
        window->onMouseButton(MouseButtons::Left, MouseButtonClick::DoubleClick);
        break;
    case WM_RBUTTONDOWN:
        window->onMouseButton(MouseButtons::Right, MouseButtonClick::Down);
        break;
    case WM_RBUTTONUP:
        window->onMouseButton(MouseButtons::Right, MouseButtonClick::Up);
        break;
    case WM_RBUTTONDBLCLK:
        window->onMouseButton(MouseButtons::Right, MouseButtonClick::DoubleClick);
        break;
    case WM_MBUTTONDOWN:
        window->onMouseButton(MouseButtons::Middle, MouseButtonClick::Down);
        break;
    case WM_MBUTTONUP:
        window->onMouseButton(MouseButtons::Middle, MouseButtonClick::Up);
        break;
    case WM_MBUTTONDBLCLK:
        window->onMouseButton(MouseButtons::Middle, MouseButtonClick::DoubleClick);
        break;
    case WM_MOUSELEAVE:
        window->onMouseLeave();
        break;
    case WM_KEYDOWN:
        [[fallthrough]];
    case WM_SYSKEYDOWN:
        [[fallthrough]];
    case WM_KEYUP:
        [[fallthrough]];
    case WM_SYSKEYUP:
        window->onKey(LOWORD(wParam), HIWORD(lParam), LOWORD(lParam));
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
