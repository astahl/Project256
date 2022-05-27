#include "MainWindow.h"
#include <Windows.h>


MainWindow::MainWindow(HWND hwnd)
    : hwnd(hwnd)
{
    this->memory = VirtualAlloc(NULL, 32L * 1024L * 1024L, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    this->drawBuffer = VirtualAlloc(0, 4 * DrawBufferHeight * DrawBufferWidth, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

MainWindow::~MainWindow()
{

}

void MainWindow::onPaint() {
    writeDrawBuffer(memory, drawBuffer);
    //HDC hdc = GetDC(hwnd);
    ValidateRect(hwnd, NULL);
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
        output = doGameThings(&input, memory);

        InvalidateRect(hwnd, NULL, FALSE);
    }
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    MainWindow& window = *reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    switch (uMsg) {
    case WM_CREATE:
        break;
    case WM_PAINT:
        window.onPaint();
        break;
    case WM_CLOSE:
        window.input.closeRequested = true;
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
