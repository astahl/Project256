

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
    
    UINT width = DrawBufferWidth;
    UINT height = DrawBufferHeight;
    UINT drawBufferAspectRatio16_16 = (width << 16) / height;
    UINT viewAspectRatio16_16 = (DrawAspectH << 16) / DrawAspectV;
    UINT invViewAspectRatio16_16 = (DrawAspectV << 16) / DrawAspectH;

    if (drawBufferAspectRatio16_16 > viewAspectRatio16_16) {
        height = (width * invViewAspectRatio16_16) >> 16;
    }
    else if (drawBufferAspectRatio16_16 < viewAspectRatio16_16) {
        width = (height * viewAspectRatio16_16) >> 16;
    }
        

    HWND windowHandle = CreateWindowEx(
        0,
        windowClass.lpszClassName,
        L"Project 256",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width * 2, height * 2,
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
