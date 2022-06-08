#include "Direct3D12View.h"
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <string>
#ifdef _DEBUG
#include <dxgidebug.h>
#endif

using namespace Microsoft::WRL;

Direct3D12View::Direct3D12View(HWND hwnd, UINT width, UINT height)
	: mHwnd(hwnd)
	, mWidth(width)
	, mHeight(height)
	, mSrvDataBeginPtr(nullptr)
	, mFrameIndex(0)
	, mFenceValues{}
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

		ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
		{
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

			DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
			{
				80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
			};
			DXGI_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
			filter.DenyList.pIDList = hide;
			dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
		}
	}
#endif

	ComPtr<IDXGIFactory7> factory;
	ExitOnFail(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	// Adapter, Device and Command Queue
	ComPtr<IDXGIAdapter4> adapter;
	for (UINT adapterIndex = 0; SUCCEEDED(factory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_UNSPECIFIED, IID_PPV_ARGS(&adapter))); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC3 desc;
		adapter->GetDesc3(&desc);
		if (desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)
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

	// Swap Chain
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {
		.Width = mWidth,
		.Height = mHeight,
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
		ExitOnFail(factory->CreateSwapChainForHwnd(mCommandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, &swapChain));
		ExitOnFail(factory->MakeWindowAssociation(hwnd, 0));
		ExitOnFail(swapChain.As(&mSwapChain));
	}
	mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();

	// descriptor heap
	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			.NumDescriptors = FrameCount,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		};

		ExitOnFail(mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap)));

		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			.NumDescriptors = 1,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		};
		ExitOnFail(mDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvHeap)));

		mRtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		mSrvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	this->CreateRenderTargetViews();

	// graphics pipeline state / shader compilation & setup
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(mDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}
		{
			D3D12_ROOT_PARAMETER1 parameterCbv{
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,
				.Constants = {
					.ShaderRegister = 0,
					.RegisterSpace = 0,
					.Num32BitValues = 2,
				},
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX,
			};

			D3D12_DESCRIPTOR_RANGE1 rangeSrv{
				.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
				.NumDescriptors = 1,
				.RegisterSpace = 0,
				.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC,
				.OffsetInDescriptorsFromTableStart = 0,
			};
			D3D12_ROOT_PARAMETER1 parameterSrv{
				.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
				.DescriptorTable {
					.NumDescriptorRanges = 1,
					.pDescriptorRanges = &rangeSrv,
				},
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
			};

			D3D12_STATIC_SAMPLER_DESC sampler{
				.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT,
				.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER,
				.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER,
				.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER,
				.MipLODBias = 0,
				.MaxAnisotropy = 0,
				.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
				.MinLOD = 0.0f,
				.MaxLOD = D3D12_FLOAT32_MAX,
				.ShaderRegister = 0,
				.RegisterSpace = 0,
				.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
			};

			D3D12_ROOT_PARAMETER1 parameters[2] = { parameterCbv, parameterSrv };

			D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc{
				.Version = D3D_ROOT_SIGNATURE_VERSION::D3D_ROOT_SIGNATURE_VERSION_1_1,
				.Desc_1_1 {
					.NumParameters = _countof(parameters),
					.pParameters = parameters,
					.NumStaticSamplers = 1,
					.pStaticSamplers = &sampler,
					.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT,
				},
			};
			ComPtr<ID3DBlob> signature;
			ComPtr<ID3DBlob> error;

			ExitOnFail(D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error));
			ExitOnFail(mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));
		}
		

		ComPtr<ID3D10Blob> vertexShader;
		ComPtr<ID3D10Blob> pixelShader;
		ComPtr<ID3D10Blob> error;
		UINT compileFlags = 0;
#ifdef _DEBUG
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		std::wstring pathBuf(512, L'\0');
		GetModuleFileName(nullptr, pathBuf.data(), 512);
		size_t lastSlashPos = pathBuf.find_last_of(L'\\');
		if (lastSlashPos != std::wstring::npos) pathBuf.erase(lastSlashPos + 1);
		std::wstring shaderPath = pathBuf + L"Shader.hlsl";
		HRESULT result = D3DCompileFromFile(shaderPath.c_str(), nullptr, nullptr, "pixelShader", "ps_5_0", compileFlags, 0, &pixelShader, &error);
		if (!SUCCEEDED(result)) {
			std::string errorMsg((char*)error.Get()->GetBufferPointer(), error.Get()->GetBufferSize());
			OutputDebugStringA(errorMsg.c_str());
			exit(1);
		}
		result = D3DCompileFromFile(shaderPath.c_str(), nullptr, nullptr, "vertexShader", "vs_5_0", compileFlags, 0, &vertexShader, &error);
		if (!SUCCEEDED(result)) {
			std::string errorMsg((char*)error.Get()->GetBufferPointer(), error.Get()->GetBufferSize());
			OutputDebugStringA(errorMsg.c_str());
			exit(1);
		}


		const int inputElementCount = 1;
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[inputElementCount] =
		{
			{ "SV_VertexID", 0, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};


		D3D12_RENDER_TARGET_BLEND_DESC defaultRtBlend{
			.BlendEnable = FALSE,
			.LogicOpEnable = FALSE,
			.SrcBlend = D3D12_BLEND_ONE,
			.DestBlend = D3D12_BLEND_ZERO,
			.BlendOp = D3D12_BLEND_OP_ADD,
			.SrcBlendAlpha = D3D12_BLEND_ONE,
			.DestBlendAlpha = D3D12_BLEND_ZERO,
			.BlendOpAlpha = D3D12_BLEND_OP_ADD,
			.LogicOp = D3D12_LOGIC_OP_NOOP,
			.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL,
		};

		D3D12_BLEND_DESC blend{
			.AlphaToCoverageEnable = FALSE,
			.IndependentBlendEnable = FALSE,
		};
		for (int i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
			blend.RenderTarget[i] = defaultRtBlend;
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{
			.pRootSignature = mRootSignature.Get(),
			.VS {
				.pShaderBytecode = vertexShader.Get()->GetBufferPointer(),
				.BytecodeLength = vertexShader.Get()->GetBufferSize(),
			},
			.PS {
				.pShaderBytecode = pixelShader.Get()->GetBufferPointer(),
				.BytecodeLength = pixelShader.Get()->GetBufferSize(),
			},
			.StreamOutput {
				.pSODeclaration = 0,
				.NumEntries = 0,
				.pBufferStrides = 0,
				.NumStrides = 0,
				.RasterizedStream = D3D12_SO_NO_RASTERIZED_STREAM,
			},
			.BlendState = blend,
			.SampleMask = UINT_MAX,
			.RasterizerState = D3D12_RASTERIZER_DESC{
				.FillMode = D3D12_FILL_MODE_SOLID,
				.CullMode = D3D12_CULL_MODE_BACK,
				.FrontCounterClockwise = FALSE,
				.DepthBias = 0,
				.DepthBiasClamp = 0.0f,
				.SlopeScaledDepthBias = 0.0f,
				.DepthClipEnable = TRUE,
				.MultisampleEnable = FALSE,
				.AntialiasedLineEnable = FALSE,
				.ForcedSampleCount = 0,
				.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF,
			},
			.DepthStencilState {
				.DepthEnable = FALSE,
				.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
				.DepthFunc = D3D12_COMPARISON_FUNC_LESS,
				.StencilEnable = FALSE,
				.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK,
				.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
				.FrontFace {
					.StencilFailOp = D3D12_STENCIL_OP_KEEP,
					.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP,
					.StencilPassOp = D3D12_STENCIL_OP_KEEP,
					.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS,
				},
				.BackFace {
					.StencilFailOp = D3D12_STENCIL_OP_KEEP,
					.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP,
					.StencilPassOp = D3D12_STENCIL_OP_KEEP,
					.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS,
				},
			},
			.InputLayout = { inputElementDescs, inputElementCount },
			.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
			.NumRenderTargets = 1,
			.RTVFormats = { DXGI_FORMAT_R8G8B8A8_UNORM },
			.SampleDesc {
				.Count = 1,
			},
			.NodeMask = 0,
			.Flags = D3D12_PIPELINE_STATE_FLAG_NONE,
		};

		ExitOnFail(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPipelineState)));

		for (int i = 0; i < FrameCount; ++i)
			ExitOnFail(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocator[i])));

		ExitOnFail(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator[mFrameIndex].Get(), mPipelineState.Get(), IID_PPV_ARGS(&mCommandList)));
		//ExitOnFail(mDevice->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&mCommandList)));

		// create Texture
		{
			D3D12_RESOURCE_DESC textureDesc = {
				.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				.Width = DrawBufferWidth,
				.Height = DrawBufferHeight,
				.DepthOrArraySize = 1,
				.MipLevels = 1,
				.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
				.SampleDesc {
					.Count = 1,
					.Quality = 0,
				},
				.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
				.Flags = D3D12_RESOURCE_FLAG_NONE,
			};
			D3D12_HEAP_PROPERTIES props{
				.Type = D3D12_HEAP_TYPE_DEFAULT,
				.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
				.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
				.CreationNodeMask = 1,
				.VisibleNodeMask = 1,
			};

			ExitOnFail(mDevice->CreateCommittedResource(
				&props,
				D3D12_HEAP_FLAG_NONE,
				&textureDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&mTexture)));

			// Create the GPU upload buffer.
			D3D12_HEAP_PROPERTIES uploadHeapProps{
				.Type = D3D12_HEAP_TYPE_UPLOAD,
				.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
				.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
				.CreationNodeMask = 1,
				.VisibleNodeMask = 1,
			};

			UINT64 uploadBufferSize = 0;
			mDevice->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &uploadBufferSize);

			D3D12_RESOURCE_DESC uploadHeapResourceDesc{
				.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
				.Alignment = 0,
				.Width = uploadBufferSize,
				.Height = 1,
				.DepthOrArraySize = 1,
				.MipLevels = 1,
				.Format = DXGI_FORMAT_UNKNOWN,
				.SampleDesc{
					.Count = 1,
					.Quality = 0,
				},
				.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
				.Flags = D3D12_RESOURCE_FLAG_NONE,
			};

			ExitOnFail(mDevice->CreateCommittedResource(
				&uploadHeapProps,
				D3D12_HEAP_FLAG_NONE,
				&uploadHeapResourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&mTextureUploadHeap)));

			// Copy data to the intermediate upload heap and then schedule a copy 
			// from the upload heap to the Texture2D.

			const size_t memSize = DrawBufferHeight * DrawBufferWidth * 4;
			mDrawBuffer = reinterpret_cast<BYTE*>(VirtualAlloc(0, memSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
			writeDrawBuffer(nullptr, mDrawBuffer);

			D3D12_SUBRESOURCE_DATA textureData{
				.pData = mDrawBuffer,
				.RowPitch = DrawBufferWidth * 4,
				.SlicePitch = memSize,
			};

			D3D12_RESOURCE_DESC textDesc = mTexture->GetDesc();
			const UINT MaxSubresources = 16;
			UINT64 requiredSize = 0;
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT layouts[MaxSubresources];
			UINT numRows[MaxSubresources];
			UINT64 rowSizesInBytes[MaxSubresources];

			mDevice->GetCopyableFootprints(&textDesc, 0, 1, 0, layouts, numRows, rowSizesInBytes, &requiredSize);
			byte* data = nullptr;
			ExitOnFail(mTextureUploadHeap->Map(0, nullptr, reinterpret_cast<void**>(&data)));
			D3D12_MEMCPY_DEST memcpydest{
				.pData = data + layouts[0].Offset,
				.RowPitch = layouts[0].Footprint.RowPitch,
				.SlicePitch = layouts[0].Footprint.RowPitch * numRows[0],
			};
			for (UINT row = 0; row < numRows[0]; ++row)
			{
				memcpy(static_cast<byte*>(memcpydest.pData) + memcpydest.RowPitch * row, mDrawBuffer + DrawBufferWidth * 4 * row, rowSizesInBytes[0]);
			}
			mTextureUploadHeap->Unmap(0, nullptr);

			// Describe and create a SRV for the texture.
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {
				.Format = textureDesc.Format,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D {
					.MipLevels = 1,
				},
			};
			mDevice->CreateShaderResourceView(mTexture.Get(), &srvDesc, mSrvHeap->GetCPUDescriptorHandleForHeapStart());

			//ExitOnFail(mCommandAllocator->Reset());
			//ExitOnFail(mCommandList->Reset(mCommandAllocator.Get(), mPipelineState.Get()));

			D3D12_TEXTURE_COPY_LOCATION srcLocation{
				.pResource = mTextureUploadHeap.Get(),
				.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
				.PlacedFootprint = layouts[0],
			};

			D3D12_TEXTURE_COPY_LOCATION dstLocation{
				.pResource = mTexture.Get(),
				.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
				.SubresourceIndex = 0
			};

			mCommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

			D3D12_RESOURCE_BARRIER barrierTextureCopyTransition{
				.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
				.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
				.Transition = {
					.pResource = mTexture.Get(),
					.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
					.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST,
					.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				},
			};
			mCommandList->ResourceBarrier(1, &barrierTextureCopyTransition);
		}

		ExitOnFail(mCommandList->Close());

		ID3D12CommandList* cmdListPtrArray[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(1, cmdListPtrArray);
	}

	// create fence
	{
		ExitOnFail(mDevice->CreateFence(mFenceValues[mFrameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
		mFenceValues[mFrameIndex]++;

		mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (mFenceEvent == nullptr) {
			ExitOnFail(HRESULT_FROM_WIN32(GetLastError()));
		}
	}
	WaitForGpu();
}

Direct3D12View::~Direct3D12View()
{
	WaitForGpu();
	CloseHandle(mFenceEvent);
}


void Direct3D12View::Resize(UINT width, UINT height)
{
	mWidth = width;
	mHeight = height;
	
	mViewport = {
	.TopLeftX = 0,
	.TopLeftY = 0,
	.Width = static_cast<FLOAT>(mWidth),
	.Height = static_cast<FLOAT>(mHeight),
	};
	mScissorRect = {
		.left = static_cast<LONG>(mViewport.TopLeftX),
		.top = static_cast<LONG>(mViewport.TopLeftY),
		.right = static_cast<LONG>(mViewport.TopLeftX + mViewport.Width),
		.bottom = static_cast<LONG>(mViewport.TopLeftY + mViewport.Height),
	};
	
	
	mConstantScale = clipSpaceDrawBufferScale(width, height);
	this->WaitForGpu();

	// destroy render targets
	for (UINT i = 0; i < FrameCount; ++i) {
		mRenderTargets[i].Reset();
		mFenceValues[i] = mFenceValues[mFrameIndex];
	}

	ExitOnFail(mSwapChain->ResizeBuffers(FrameCount, mWidth, mHeight, DXGI_FORMAT_UNKNOWN, 0));
	mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();
	
	this->CreateRenderTargetViews();
}

void Direct3D12View::Draw()
{
	ExitOnFail(mCommandAllocator[mFrameIndex]->Reset());
	ExitOnFail(mCommandList->Reset(mCommandAllocator[mFrameIndex].Get(), mPipelineState.Get()));
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { mSrvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	mCommandList->SetGraphicsRoot32BitConstants(0, 2, &mConstantScale, 0);
	mCommandList->SetGraphicsRootDescriptorTable(1, mSrvHeap->GetGPUDescriptorHandleForHeapStart());

	D3D12_RESOURCE_BARRIER barrierRenderTargetTransition {
		.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
		.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
		.Transition = {
			.pResource = mRenderTargets[mFrameIndex].Get(),
			.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
			.StateBefore = D3D12_RESOURCE_STATE_PRESENT,
			.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET,
		},
	};
	mCommandList->ResourceBarrier(1, &barrierRenderTargetTransition);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{
		.ptr = SIZE_T(mRtvHeap->GetCPUDescriptorHandleForHeapStart().ptr) + SIZE_T(mFrameIndex * mRtvDescriptorSize),
	};

	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	mCommandList->RSSetViewports(1, &mViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);
	mCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	mCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	mCommandList->IASetVertexBuffers(0, 0, nullptr);
	mCommandList->DrawInstanced(4, 1, 0, 0);

	barrierRenderTargetTransition.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierRenderTargetTransition.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	mCommandList->ResourceBarrier(1, &barrierRenderTargetTransition);
	ExitOnFail(mCommandList->Close());

	ID3D12CommandList* cmdListPtrArray[] = {mCommandList.Get()};

	mCommandQueue->ExecuteCommandLists(1, cmdListPtrArray);
	ExitOnFail(mSwapChain->Present(1, 0));

	MoveToNextFrame();
}

void Direct3D12View::WaitForGpu()
{
	// Schedule a Signal command in the queue.
	ExitOnFail(mCommandQueue->Signal(mFence.Get(), mFenceValues[mFrameIndex]));
	// Wait until the fence has been processed.
	ExitOnFail(mFence->SetEventOnCompletion(mFenceValues[mFrameIndex], mFenceEvent));
	WaitForSingleObjectEx(mFenceEvent, INFINITE, FALSE);

	// Increment the fence value for the current frame.
	mFenceValues[mFrameIndex]++;
}


void Direct3D12View::MoveToNextFrame()
{
	// Schedule a Signal command in the queue.
	const UINT64 currentFenceValue = mFenceValues[mFrameIndex];
	ExitOnFail(mCommandQueue->Signal(mFence.Get(), currentFenceValue));

	// Update the frame index.
	mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	if (mFence->GetCompletedValue() < mFenceValues[mFrameIndex])
	{
		ExitOnFail(mFence->SetEventOnCompletion(mFenceValues[mFrameIndex], mFenceEvent));
		WaitForSingleObjectEx(mFenceEvent, INFINITE, FALSE);
	}

	// Set the fence value for the next frame.
	mFenceValues[mFrameIndex] = currentFenceValue + 1;
}

void Direct3D12View::CreateRenderTargetViews()
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < FrameCount; ++i) {
		ExitOnFail(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mRenderTargets[i])));
		mDevice->CreateRenderTargetView(mRenderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.ptr += mRtvDescriptorSize;
	}
}

bool Direct3D12View::ExitOnFail(HRESULT result) {
	if (SUCCEEDED(result)) {
		return true;
	}
	exit(result);
}