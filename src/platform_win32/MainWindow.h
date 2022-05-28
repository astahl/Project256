#pragma once
#include <Windows.h>
#include <memory>


#define CXX
#include "../game/Project256.h"

class Direct3D12View;

class MainWindow
{
    HWND hwnd;
    GameInput input;
    std::unique_ptr<Direct3D12View> view;
    std::unique_ptr<uint8_t> memory;
    std::unique_ptr<uint8_t> drawBuffer;

    void onPaint();

public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    MainWindow(HWND hwnd);
    ~MainWindow();

    void doMainLoop();
};

