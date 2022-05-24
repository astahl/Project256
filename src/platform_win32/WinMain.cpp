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
        0,
        windowClass.lpszClassName,
        L"Project 256",    // Window text
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        instanceHandle,
        NULL
    );

    if (windowHandle == NULL)
    {
        return 0;
    }
    
    ShowWindow(windowHandle, nCmdShow);

    MSG message = { };
    while (GetMessage(&message, NULL, 0, 0) > 0)
    {
        if (message.message == WM_QUIT) {
            break;
        }
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
