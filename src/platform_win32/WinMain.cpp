

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

    MainWindow* windowPtr{};
    MainWindow** windowPtrPtr = &windowPtr;
    
    HWND windowHandle = CreateWindowEx(
        0,
        windowClass.lpszClassName,
        L"Project 256",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, DrawBufferWidth * 2, DrawBufferHeight * 2,
        NULL,
        NULL,
        instanceHandle,
        reinterpret_cast<LPVOID>(windowPtrPtr)
    );

    if (!windowHandle)
        return 0;
    
    ShowWindow(windowHandle, nCmdShow);
    windowPtr->doMainLoop();

    return 0;
}
