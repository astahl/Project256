#include "MainWindow.h"
#include "Direct3D12View.h"


MainWindow::MainWindow(HWND hwnd)
    : hwnd(hwnd)
{
    this->memory = reinterpret_cast<uint8_t*>(VirtualAlloc(NULL, MemorySize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    this->drawBuffer = reinterpret_cast<uint8_t*>(VirtualAlloc(0, 4 * DrawBufferHeight * DrawBufferWidth, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));

    RECT rect{};
    GetWindowRect(hwnd, &rect);
    this->view = new Direct3D12View(hwnd, rect.right - rect.left, rect.bottom - rect.top);
}

MainWindow::~MainWindow()
{

}

void MainWindow::onPaint() {
    writeDrawBuffer(memory, drawBuffer);
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

    GameOutput output{};
    while (!output.shouldQuit)
    {
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        Sleep(20);
        output = doGameThings(&input, memory);

        InvalidateRect(hwnd, NULL, FALSE);
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
        window->input.closeRequested = BOOLEAN_ENUM::eTRUE;
        break;
    case WM_DESTROY: {
        //delete window;
    } break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
