#include "Direct3D12View.h"
#include <dxgi1_6.h>

using namespace Microsoft::WRL;

Direct3D12View::Direct3D12View(HWND hwnd)
	: mHwnd(hwnd)
{
	UINT dxgiFactoryFlags = 0;

#ifdef _DEBUG
// Enable the debug layer (requires the Graphics Tools "optional feature").
// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	ComPtr<IDXGIFactory7> factory;
	ExitOnFail(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	// get the adapter
	ComPtr<IDXGIAdapter4> adapter;
	for (UINT adapterIndex = 0; SUCCEEDED(factory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_UNSPECIFIED, IID_PPV_ARGS(&adapter))); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC3 desc;
		adapter->GetDesc3(&desc);
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr))) {
			break;
		}
	}
	{
		ComPtr<ID3D12Device> device;
		ExitOnFail(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));
		ExitOnFail(device.As(&mDevice));
	}
	D3D12_COMMAND_QUEUE_DESC queueDesc = {
		.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
		.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
	};

	ExitOnFail(mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {
		.Width = 200,
		.Height = 200,
		.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
		.SampleDesc = {
			.Count = 1,
		},
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = FrameCount,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
	};

	{
		ComPtr<IDXGISwapChain1> swapChain;
		// TODO no fullscreen desc?
		ExitOnFail(factory->CreateSwapChainForHwnd(mCommandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, &swapChain));
		// TODO no fullscreen?
		ExitOnFail(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
		ExitOnFail(swapChain.As(&mSwapChain));
	}
	mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();

	// descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = FrameCount,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
	};

	ExitOnFail(mDevice->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&mRtvHeap)));
	mRtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// frame resources

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < FrameCount; ++i) {
		ExitOnFail(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mRenderTargets[i])));
		mDevice->CreateRenderTargetView(mRenderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.ptr += mRtvDescriptorSize;
	}
	ExitOnFail(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocator)));
	ExitOnFail(mDevice->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&mCommandList)));
	
	ExitOnFail(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
	mFenceValue = 1;
	mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (mFenceEvent == nullptr) {
		ExitOnFail(HRESULT_FROM_WIN32(GetLastError()));
	}
}

Direct3D12View::~Direct3D12View()
{
	WaitForPreviousFrame();
	CloseHandle(mFenceEvent);
}

void Direct3D12View::Draw()
{
	ExitOnFail(mCommandAllocator->Reset());
	ExitOnFail(mCommandList->Reset(mCommandAllocator.Get(), mPipelineState.Get()));

	D3D12_RESOURCE_BARRIER barrier1{
		.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
		.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
		.Transition = {
			.pResource = mRenderTargets[mFrameIndex].Get(),
			.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
			.StateBefore = D3D12_RESOURCE_STATE_PRESENT,
			.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET,
		},
	};
	mCommandList->ResourceBarrier(1, &barrier1);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{
		.ptr = SIZE_T(mRtvHeap->GetCPUDescriptorHandleForHeapStart().ptr) + SIZE_T(mFrameIndex * mRtvDescriptorSize),
	};

	const float clearColor[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	mCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	D3D12_RESOURCE_BARRIER barrier2{
		.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
		.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
		.Transition = {
			.pResource = mRenderTargets[mFrameIndex].Get(),
			.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
			.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET,
			.StateAfter = D3D12_RESOURCE_STATE_PRESENT,
		},
	};

	mCommandList->ResourceBarrier(1, &barrier2);
	ExitOnFail(mCommandList->Close());

	ID3D12CommandList* cmdListPtrArray[] = {mCommandList.Get()};

	mCommandQueue->ExecuteCommandLists(1, cmdListPtrArray);
	ExitOnFail(mSwapChain->Present(1, 0));
	WaitForPreviousFrame();
}

void Direct3D12View::WaitForPreviousFrame()
{
	// TODO this is not good it seems??
	const UINT64 fence = mFenceValue;
	ExitOnFail(mCommandQueue->Signal(mFence.Get(), fence));
	mFenceValue++;

	// Wait until the previous frame is finished.
	if (mFence->GetCompletedValue() < fence)
	{
		ExitOnFail(mFence->SetEventOnCompletion(fence, mFenceEvent));
		WaitForSingleObject(mFenceEvent, INFINITE);
	}

	mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();
}

bool Direct3D12View::ExitOnFail(HRESULT result) {
	if (SUCCEEDED(result)) {
		return true;
	}
	exit(result);
}