#pragma once
#include <Windows.h>
#include <cstdint>


class Direct3D12View;

struct GameState; 

class MainWindow
{
    GameState* state;
    HWND hwnd;
    Direct3D12View* view;
    byte* memory;
    byte* drawBuffer;

    void onPaint();
    void onResize();
    void onTick();
public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    MainWindow(HWND hwnd);
    ~MainWindow();

    void doMainLoop();
};

