#pragma once
#include <Windows.h>

#define CXX
#include "../game/Project256.h"

class MainWindow
{
    HWND hwnd;
    GameInput input;
    void* memory;
    void* drawBuffer;

    void onPaint();

public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    MainWindow(HWND hwnd);
    ~MainWindow();

    void doMainLoop();
};

