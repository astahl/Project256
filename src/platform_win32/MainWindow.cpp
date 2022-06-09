#include "MainWindow.h"
#include "Direct3D12View.h"

#define CXX
#include "../game/Project256.h"


class Chronometer {
    LARGE_INTEGER frequency = {};
    int lastTimeIndex = 0;
    LARGE_INTEGER timevalues[2] = {};
public:
    struct Time {
        INT64 microseconds;
        double seconds;
    };

    Time elapsed() {
        int nextTimeIndex = (this->lastTimeIndex == 0 ? 1 : 0);
        QueryPerformanceCounter(&timevalues[nextTimeIndex]);
        auto t0 = timevalues[lastTimeIndex];
        auto t1 = timevalues[nextTimeIndex];
        this->lastTimeIndex = nextTimeIndex;
        LARGE_INTEGER elapsedMicroseconds{};
        elapsedMicroseconds.QuadPart = ((t1.QuadPart - t0.QuadPart) * 1'000'000) / frequency.QuadPart;
        return Time{
        .microseconds = elapsedMicroseconds.QuadPart,
        .seconds = double(elapsedMicroseconds.QuadPart) / 1'000'000
        };
    }

    Chronometer() {
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&timevalues[lastTimeIndex]);
    }
};

struct GameState {
    uint64_t frameCount;
    int64_t upTime{};
    GameInput input;
    Chronometer frameTime{};
};

enum class Timers : UINT_PTR {
    HighFrequency
};


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
}

MainWindow::~MainWindow()
{
    KillTimer(hwnd, static_cast<UINT_PTR>(Timers::HighFrequency));
}

void MainWindow::onTick() {
    GameInput& input = state->input;

    bool mouseWasOver = input.mouse.endedOver == eTRUE && input.mouse.trackLength > 0;
    Vec2f lastMousePosition{};
    if (mouseWasOver) {
        lastMousePosition = input.mouse.track[input.mouse.trackLength - 1];
    }
    state->input.hasMouse = eTRUE;

    state->input.frameNumber = state->frameCount++;
    auto frameTime = state->frameTime.elapsed();
    state->upTime += frameTime.microseconds;
    state->input.elapsedTime_s = frameTime.seconds;
    state->input.upTime_microseconds = state->upTime;;

    GameOutput output{};
    output = doGameThings(&state->input, memory);
    state->input = {};
    if (output.shouldQuit) {
        PostQuitMessage(0);
        return;
    }
    if (mouseWasOver) {
        input.mouse.track[0] = lastMousePosition;
        ++input.mouse.trackLength;
        input.mouse.endedOver = eTRUE;
    }

    InvalidateRect(hwnd, NULL, FALSE);
}

void MainWindow::onPaint() {
    writeDrawBuffer(memory, drawBuffer);
    this->view->SetDrawBuffer(drawBuffer);
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
    if (scaledPos.x < 0.0f || scaledPos.x > 1.0f || scaledPos.y < 0.0f || scaledPos.x > 1.0f) {
        // outside
        mouse.endedOver = eFALSE;
    } else {
        auto pixelPos = Vec2f{ .x = scaledPos.x * DrawBufferWidth, .y = scaledPos.y * DrawBufferHeight };
        mouse.track[mouse.trackLength++] = pixelPos;
        mouse.endedOver = eTRUE;
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
    case WM_DESTROY: {
        //delete window;
    } break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
