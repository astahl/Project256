#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>


using Microsoft::WRL::ComPtr;

class Direct3D12View {
    static const UINT FrameCount = 2;
	HWND mHwnd; 

	ComPtr<ID3D12Device4> mDevice;
	ComPtr<ID3D12CommandQueue> mCommandQueue;
	ComPtr<IDXGISwapChain4> mSwapChain;
	UINT mFrameIndex;

    // Pipeline objects.
    ComPtr<ID3D12Resource> mRenderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> mCommandAllocator;
    ComPtr<ID3D12DescriptorHeap> mRtvHeap;
    ComPtr<ID3D12PipelineState> mPipelineState;
    ComPtr<ID3D12GraphicsCommandList> mCommandList;
    UINT mRtvDescriptorSize;

    // Synchronization objects.
    HANDLE mFenceEvent;
    ComPtr<ID3D12Fence> mFence;
    UINT64 mFenceValue;

    void WaitForPreviousFrame();
	static bool ExitOnFail(HRESULT result);

public:
	Direct3D12View(HWND hwnd);
	~Direct3D12View();

    void Draw(); 
};