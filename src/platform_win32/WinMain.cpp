

#ifndef UNICODE
#define UNICODE
#endif 
#include <windows.h>
#include <functional>


#include "MainWindow.h"
// declarations



// Structs
struct Drawing {
    void* buffer;
};



int WINAPI wWinMain(_In_ HINSTANCE instanceHandle,
    _In_opt_ HINSTANCE previousInstance,
    _In_ LPWSTR commandLine,
    _In_ int nCmdShow) 
{
 
    auto windowProc = (&MainWindow::WindowProc);
    WNDCLASSEX windowClass {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_VREDRAW | CS_HREDRAW,
        .lpfnWndProc   = windowProc,
        .hInstance     = instanceHandle,
        .lpszClassName = L"Project256 Window Class"
    };

    RegisterClassEx(&windowClass);

    HWND windowHandle = CreateWindowEx(
        0,
        windowClass.lpszClassName,
        L"Project 256",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, DrawBufferWidth * 2, DrawBufferHeight * 2,
        NULL,
        NULL,
        instanceHandle,
        NULL
    );

    if (!windowHandle)
        return 0;
    
    MainWindow window{windowHandle};
    SetWindowLongPtr(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&window));

    ShowWindow(windowHandle, nCmdShow);
    window.doMainLoop();

    return 0;
}
