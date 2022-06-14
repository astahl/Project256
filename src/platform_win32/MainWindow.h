#pragma once
#include <Windows.h>
#include <cstdint>


class Direct3D12View;

struct GameState; 

enum class MouseButtons { Left, Right, Middle };
enum class MouseButtonClick { Down, Up, DoubleClick };

class MainWindow
{
    const static int PROFILING_STR_BUFFER_LENGTH = 1000;
    GameState* state;
    HWND hwnd;
    Direct3D12View* view;
    char profilingStringBuffer[PROFILING_STR_BUFFER_LENGTH];

    void onPaint();
    void onResize();
    void onTick();
    void onClose();
    void onTimer(WPARAM timerId);
    void onMouseMove(POINTS points);
    void onMouseLeave();
    void onMouseButton(MouseButtons button, MouseButtonClick clickType);
public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    MainWindow(HWND hwnd);
    ~MainWindow();

    int doMainLoop();
};

