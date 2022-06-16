#pragma once
#include <Windows.h>
#include <cstdint>
#include "../game/defines.h"

class Direct3D12View;

struct GameState; 

enum class MouseButtons { Left, Right, Middle };
enum class MouseButtonClick { Down, Up, DoubleClick };

class MainWindow
{
    constant int PROFILING_STR_BUFFER_LENGTH = 1000;
    GameState* mGameState;
    HWND mHwnd;
    Direct3D12View* mGameView;
    char profilingStringBuffer[PROFILING_STR_BUFFER_LENGTH]{};

    void onPaint();
    void onResize();
    void onTick();
    void onClose();
    void onTimer(WPARAM timerId);
    void onMouseMove(POINTS points);
    void onMouseLeave();
    void onMouseButton(MouseButtons button, MouseButtonClick clickType);
    void onKey(WORD vkey, WORD flags, WORD repeatCount);
    void onActiveChange(bool willBeActive);
public:
    classmethod LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    MainWindow(HWND hwnd);
    ~MainWindow();

    int doMainLoop();
};

