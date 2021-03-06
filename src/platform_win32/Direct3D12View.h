#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <thread>

#include "../game/Project256.h"

using Microsoft::WRL::ComPtr;

class Direct3D12View {

    static const UINT FrameCount = 2;
	HWND mHwnd; 
    UINT mWidth;
    UINT mHeight;

	ComPtr<ID3D12Device4> mDevice;
	ComPtr<ID3D12CommandQueue> mCommandQueue;
	ComPtr<IDXGISwapChain4> mSwapChain;
	UINT mFrameIndex;

    std::jthread mPresentProfilingThread;

    D3D12_VIEWPORT mViewport;
    D3D12_RECT mScissorRect;

    // resources
    ComPtr<ID3D12Resource> mTexture;
    ComPtr<ID3D12Resource> mTextureUploadHeap;
    UINT8* mSrvDataBeginPtr;
    BYTE* mDrawBuffer;
    bool mNeedsUpload;

    Vec2f mConstantScale;

    // Pipeline objects.
    ComPtr<ID3D12Resource> mRenderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> mCommandAllocator[FrameCount];
    ComPtr<ID3D12DescriptorHeap> mRtvHeap;
    ComPtr<ID3D12DescriptorHeap> mSrvHeap;
    ComPtr<ID3D12PipelineState> mPipelineState;
    ComPtr<ID3D12GraphicsCommandList> mCommandList;
    ComPtr<ID3D12RootSignature> mRootSignature;
    UINT mRtvDescriptorSize;
    UINT mSrvDescriptorSize;

    // Synchronization objects.
    HANDLE mFenceEvent;
    ComPtr<ID3D12Fence> mFence;
    UINT64 mFenceValues[FrameCount];

    void WaitForGpu();
    void MoveToNextFrame();
    void CreateRenderTargetViews();

	static bool ExitOnFail(HRESULT result);

public:
	Direct3D12View(HWND hwnd, UINT width, UINT height);
	~Direct3D12View();

    Vec2f currentScale() const;
    void Resize(UINT width, UINT height);
    void SetDrawBuffer(byte* drawBuffer);
    void Draw(); 
};