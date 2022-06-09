

#ifndef UNICODE
#define UNICODE
#endif 
#include <windows.h>
#include <functional>


#include "MainWindow.h"


int WINAPI wWinMain(_In_ HINSTANCE instanceHandle,
    _In_opt_ HINSTANCE /* previousInstance */,
    _In_ LPWSTR /* commandLine */,
    _In_ int nCmdShow) 
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
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
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
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
