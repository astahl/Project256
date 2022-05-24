#include "../game/Project256.cpp"

#ifndef UNICODE
#define UNICODE
#endif 
#include <windows.h>


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(_In_ HINSTANCE instanceHandle,
    _In_opt_ HINSTANCE previousInstance,
    _In_ LPWSTR commandLine,
    _In_ int nCmdShow) 
{
    WNDCLASS windowClass {
        .lpfnWndProc   = WindowProc,
        .hInstance     = instanceHandle,
        .lpszClassName = L"Project256 Window Class"
    };

    RegisterClass(&windowClass);

    HWND windowHandle = CreateWindowEx(
        0,                              // Optional window styles.
        windowClass.lpszClassName,
        L"Project 256",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        instanceHandle,
        NULL        // Additional application data
    );

    if (windowHandle == NULL)
    {
        return 0;
    }
    
    ShowWindow(windowHandle, nCmdShow);

    MSG message = { };
    while (GetMessage(&message, NULL, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
