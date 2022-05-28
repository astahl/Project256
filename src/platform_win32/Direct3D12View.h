#pragma once
#include <Windows.h>


class Direct3D12View {
	HWND hwnd;

public:
	Direct3D12View(HWND hwnd);
	~Direct3D12View();
};