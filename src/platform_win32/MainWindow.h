#pragma once
#include <Windows.h>
#include <cstdint>

#define CXX
#include "../game/Project256.h"

class Direct3D12View;

class MainWindow
{
    HWND hwnd;
    GameInput input;
    Direct3D12View* view;
    uint8_t* memory;
    uint8_t* drawBuffer;

    void onPaint();

public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    MainWindow(HWND hwnd);
    ~MainWindow();

    void doMainLoop();
};

