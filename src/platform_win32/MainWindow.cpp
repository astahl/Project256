#include "MainWindow.h"
#include "Direct3D12View.h"
#include <initializer_list>
#define CXX
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
    this->memory = reinterpret_cast<byte*>(VirtualAlloc(NULL, MemorySize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    this->drawBuffer = reinterpret_cast<byte*>(VirtualAlloc(0, 4 * DrawBufferHeight * DrawBufferWidth, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
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
    profiling_time_set(&GameState::timingData, eTimerTick);
    state->input.hasMouse = eTRUE;

    state->input.frameNumber = state->frameCount++;
    auto frameTime = state->frameTime.elapsed();
    state->upTime += frameTime.microseconds;
    state->input.elapsedTime_s = frameTime.seconds;
    state->input.upTime_microseconds = state->upTime;;

    GameInput inputCopy = state->input;
    state->input = {};
    GameOutput output{};

    profiling_time_interval(&GameState::timingData, eTimerTick, eTimingTickSetup);
    output = doGameThings(&inputCopy, memory);
    profiling_time_interval(&GameState::timingData, eTimerTick, eTimingTickDo);
  
    if (output.shouldQuit) {
        PostQuitMessage(0);
        return;
    }

    if ((output.shouldShowSystemCursor == eTRUE) != state->forceCursor) {
        state->forceCursor = output.shouldShowSystemCursor == eTRUE;
        setCursorVisible(state->forceCursor);
    }

    if (output.shouldPinMouse) {
        RECT rect{};
        GetClientRect(hwnd, &rect);
        POINT center{ .x = rect.left + (rect.right - rect.left) / 2, .y = rect.top + (rect.bottom - rect.top) / 2 };
        ClientToScreen(hwnd, &center);
        SetCursorPos(center.x, center.y);
    }

    if (inputCopy.mouse.trackLength && inputCopy.mouse.endedOver) {
        state->input.mouse.track[0] = inputCopy.mouse.track[inputCopy.mouse.trackLength - 1];
        state->input.mouse.trackLength += 1;
        state->input.mouse.endedOver = eTRUE;
    }
    state->input.mouse.buttonLeft.endedDown = inputCopy.mouse.buttonLeft.endedDown;
    state->input.mouse.buttonRight.endedDown = inputCopy.mouse.buttonRight.endedDown;
    state->input.mouse.buttonMiddle.endedDown = inputCopy.mouse.buttonMiddle.endedDown;
    InvalidateRect(hwnd, NULL, FALSE);

    profiling_time_interval(&GameState::timingData, eTimerTick, eTimingTickPost);
}

void MainWindow::onPaint() {
    profiling_time_set(&GameState::timingData, eTimerBufferCopy);
    writeDrawBuffer(memory, drawBuffer);
    this->view->SetDrawBuffer(drawBuffer);
    profiling_time_interval(&GameState::timingData, eTimerBufferCopy, eTimingBufferCopy);
    this->view->Draw();
    ValidateRect(hwnd, NULL);
}

void MainWindow::onResize() {
    RECT rect{};
    GetWindowRect(hwnd, &rect);
    
    this->view->Resize(rect.right - rect.left, rect.bottom - rect.top);
}

void MainWindow::onClose() {
    this->state->input.closeRequested = boole::eTRUE;
}

void MainWindow::onTimer(WPARAM timerId) {
    switch (static_cast<Timers>(timerId)) {
    case Timers::HighFrequency:
        this->onTick();
        break;
    case Timers::LowFrequency:
        profiling_time_print(&GameState::timingData);
        profiling_time_clear(&GameState::timingData);
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
        mouse.endedOver = eFALSE;
    } else {
        setCursorVisible(state->forceCursor);
        auto pixelPos = Vec2f{ .x = scaledPos.x * DrawBufferWidth, .y = scaledPos.y * DrawBufferHeight };
        mouse.track[mouse.trackLength++] = pixelPos;
        mouse.endedOver = eTRUE;
    }
    mouse.relativeMovement.x += static_cast<float>(points.x - this->state->lastCursorPosition.x) / scale.x;
    mouse.relativeMovement.y -= static_cast<float>(points.y - this->state->lastCursorPosition.y) / scale.y;
    this->state->lastCursorPosition = Vec2i{ .x = points.x, .y = points.y };
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
        btn.endedDown = eTRUE;
        break;
    case MouseButtonClick::Up:
        btn.transitionCount++;
        btn.endedDown = eFALSE;
        break;
    case MouseButtonClick::DoubleClick:
        break;
    }
}

void MainWindow::doMainLoop() {
    MSG message = { };

    bool quitWasPosted = false;
    while (!quitWasPosted)
    {
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);

            quitWasPosted |= message.message == WM_QUIT;
        }
    }
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
        window->onClose();
        break;
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
    case WM_DESTROY: {
        //delete window;
    } break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
