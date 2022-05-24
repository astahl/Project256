#include "../game/Project256.h"

#ifndef UNICODE
#define UNICODE
#endif 
#include <windows.h>
#include <functional>

// declarations



// Structs
struct Drawing {
    void* buffer;
};

struct Window {
    GameInput input;
    void* memory;

    Drawing drawing;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};


int WINAPI wWinMain(_In_ HINSTANCE instanceHandle,
    _In_opt_ HINSTANCE previousInstance,
    _In_ LPWSTR commandLine,
    _In_ int nCmdShow) 
{
    Window window{
        .memory = VirtualAlloc(0, 32L * 1024L * 1024L, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)
    };

    window.drawing.buffer = VirtualAlloc(0, 4 * DrawBufferHeight * DrawBufferWidth, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    auto windowProc = (&Window::WindowProc);
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
    
    SetWindowLongPtr(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&window));

    ShowWindow(windowHandle, nCmdShow);

    MSG message = { };
    
    GameOutput output{};
    while (!output.shouldQuit)
    {
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        output = doGameThings(&window.input, window.memory);
        
        InvalidateRect(windowHandle, NULL, FALSE);
    }

    return 0;
}

LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    Window& window = *reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    switch (uMsg) {
    case WM_CREATE:
        break;
    case WM_PAINT:
        writeDrawBuffer(window.memory, window.drawing.buffer);
        //HDC hdc = GetDC(hwnd);
        ValidateRect(hwnd, NULL);
        break;
    case WM_CLOSE:
        window.input.closeRequested = true;
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
