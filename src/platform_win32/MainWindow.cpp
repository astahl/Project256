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


MainWindow::MainWindow(HWND hwnd)
    : state(new GameState())
    , hwnd(hwnd)
{
    this->memory = reinterpret_cast<byte*>(VirtualAlloc(NULL, MemorySize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    this->drawBuffer = reinterpret_cast<byte*>(VirtualAlloc(0, 4 * DrawBufferHeight * DrawBufferWidth, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));

    RECT rect{};
    GetWindowRect(hwnd, &rect);
    this->view = new Direct3D12View(hwnd, rect.right - rect.left, rect.bottom - rect.top);
}

MainWindow::~MainWindow()
{

}

void MainWindow::onTick() {

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
        Sleep(10);
        onTick();
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
        window->state->input.closeRequested = boole::eTRUE;
        break;
    case WM_DESTROY: {
        //delete window;
    } break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
