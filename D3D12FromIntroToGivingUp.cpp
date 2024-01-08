#include "D3D12FromIntroToGivingUp.h"

D3D12FromIntroToGivingUp::D3D12FromIntroToGivingUp(UINT width, UINT height, std::wstring name) :
    D3D12App(width, height, name) {
}

void D3D12FromIntroToGivingUp::OnInit() {
    LoadPipeline();
    LoadAssets();
}

void D3D12FromIntroToGivingUp::LoadPipeline() {
	UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG)
	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif

	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory)));
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr, // 默认适配器
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_device)
	);
	if (FAILED(hardwareResult)) {
		// 回退至WARP设备
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			warpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
		));
	}
	m_device->SetName(L"m_device");

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
	m_commandQueue->SetName(L"m_commandQueue");

	// 检测对4X MSAA质量级别的支持
	/*D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = m_backBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(m_device->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)
	));
	m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");*/

	/*DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = frameCount;
	swapChainDesc.BufferDesc.Width = m_width;
	swapChainDesc.BufferDesc.Height = m_height;
	swapChainDesc.BufferDesc.Format = m_backBufferFormat;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = m_4xMsaaEnabled ? 4 : 1;
	swapChainDesc.SampleDesc.Quality = m_4xMsaaEnabled ? (m_4xMsaaQuality - 1) : 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow = Win32App::GetHwnd();
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	ComPtr<IDXGISwapChain> swapChain;
	ThrowIfFailed(m_dxgiFactory->CreateSwapChain(
		m_commandQueue.Get(),
		&swapChainDesc,
		&swapChain
	));
	ThrowIfFailed(swapChain.As(&m_swapChain));*/
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = frameCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = m_backBufferFormat;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;
	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(m_dxgiFactory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),
		Win32App::GetHwnd(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));
	ThrowIfFailed(swapChain.As(&m_swapChain));
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = frameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		//rtvHeapDesc.NodeMask = 0;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
		m_rtvHeap->SetName(L"m_rtvHeap");
		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		/*D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask = 0;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
		m_dsvHeap->SetName(L"m_dsvHeap");
		m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);*/

		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
		cbvHeapDesc.NumDescriptors = 1;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		//cbvHeapDesc.NodeMask = 0;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));
		m_cbvHeap->SetName(L"m_cbvHeap");
		m_cbvSrvUavDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	// 创建帧资源（单个RTV实例）
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT i = 0; i < frameCount; i++) {
			ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
			m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHeapHandle);
			rtvHeapHandle.Offset(1, m_rtvDescriptorSize);
		}

		/*D3D12_RESOURCE_DESC depthStencilDesc;
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;
		depthStencilDesc.Width = m_width;
		depthStencilDesc.Height = m_height;
		depthStencilDesc.DepthOrArraySize = 1;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.Format = m_depthStencilFormat;
		depthStencilDesc.SampleDesc.Count = m_4xMsaaEnabled ? 4 : 1;
		depthStencilDesc.SampleDesc.Quality = m_4xMsaaEnabled ? (m_4xMsaaQuality - 1) : 0;
		depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE optClear;
		optClear.Format = m_depthStencilFormat;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;
		CD3DX12_HEAP_PROPERTIES pHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
		ThrowIfFailed(m_device->CreateCommittedResource(
			&pHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&optClear,
			IID_PPV_ARGS(&m_depthStencil)
		));
		m_depthStencil->SetName(L"m_depthStencil");

		// 利用此资源的格式，为整个资源的第0 mip层创建描述符
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = m_depthStencilFormat;
		dsvDesc.Texture2D.MipSlice = 0;
		m_device->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());*/
	}

	//OnResize();

	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
	m_commandAllocator->SetName(L"m_commandAllocator");
}

// https://learn.microsoft.com/en-us/windows/win32/direct3d12/creating-a-basic-direct3d-12-component
void D3D12FromIntroToGivingUp::LoadAssets() {
	{
		// Shader programs typically require resources as input (constant buffers, textures, samplers).  The root signature defines the resources the shader programs expect.  If we think of the shader programs as a function, and the input resources as function parameters, then the root signature can be thought of as defining the function signature.  
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;

		// Create a single descriptor table of CBVs.
		CD3DX12_DESCRIPTOR_RANGE1 cbvTable;
		CD3DX12_ROOT_PARAMETER1 rootParameters[1];
		cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		rootParameters[0].InitAsDescriptorTable(1, &cbvTable);

		// Allow input layout and deny uneccessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
		m_rootSignature->SetName(L"m_rootSignature");
	}

	{
#if defined(_DEBUG)
		// 使用图形调试工具启用更好的着色器调试
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
		ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		//psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = m_backBufferFormat;
		psoDesc.SampleDesc.Count = 1;
		/*psoDesc.SampleDesc.Count = m_4xMsaaEnabled ? 4 : 1;
		psoDesc.SampleDesc.Quality = m_4xMsaaEnabled ? (m_4xMsaaQuality - 1) : 0;
		psoDesc.DSVFormat = m_depthStencilFormat;*/
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
	}

	ThrowIfFailed(m_device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_commandAllocator.Get(),
		m_pipelineState.Get(),
		IID_PPV_ARGS(&m_commandList)
	));
	m_commandList->SetName(L"m_commandList");
	ThrowIfFailed(m_commandList->Close());

	// 创建顶点/索引缓冲区
	{
		Vertex vertices[] = {
			{ XMFLOAT3(-0.25f, +0.25f * m_aspectRatio, 0.0f), XMFLOAT4(Colors::Red) },
			{ XMFLOAT3(+0.25f, -0.25f * m_aspectRatio, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-0.25f, -0.25f * m_aspectRatio, 0.0f), XMFLOAT4(Colors::Blue) },
			{ XMFLOAT3(+0.25f, +0.25f * m_aspectRatio, 0.0f), XMFLOAT4(Colors::Yellow) },
		};
		uint16_t indices[] = { // must be clockwise
			0, 1, 2,
			0, 3, 1,
		};

		MeshGeometry* geometry = new MeshGeometry();
		geometry->name = "square";
		geometry->vertexBufferSize = sizeof(vertices);
		geometry->indexBufferSize = sizeof(indices);
		geometry->vertexStride = sizeof(Vertex);

		/*ThrowIfFailed(D3DCreateBlob(geometry->vertexBufferSize, &geometry->vertexBufferCPU));
		CopyMemory(geometry->vertexBufferCPU->GetBufferPointer(), vertices, geometry->vertexBufferSize);
		ThrowIfFailed(D3DCreateBlob(geometry->indexBufferSize, &geometry->indexBufferCPU));
		CopyMemory(geometry->indexBufferCPU->GetBufferPointer(), indices, geometry->indexBufferSize);
		geometry->vertexBufferGPU = D3D12Utils::CreateDefaultBuffer(m_device.Get(),
			m_commandList.Get(), vertices, geometry->vertexBufferSize, geometry->vertexBufferUploader);
		geometry->indexBufferGPU = D3D12Utils::CreateDefaultBuffer(m_device.Get(),
			m_commandList.Get(), indices, geometry->indexBufferSize, geometry->indexBufferUploader);*/

		CD3DX12_HEAP_PROPERTIES hpUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto vertexResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(geometry->vertexBufferSize);
		ThrowIfFailed(m_device->CreateCommittedResource(
			&hpUpload,
			D3D12_HEAP_FLAG_NONE,
			&vertexResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&(geometry->vertexBuffer))));
		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
		ThrowIfFailed(geometry->vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, vertices, sizeof(vertices));
		geometry->vertexBuffer->Unmap(0, nullptr);

		auto indexResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(geometry->indexBufferSize);
		ThrowIfFailed(m_device->CreateCommittedResource(
			&hpUpload,
			D3D12_HEAP_FLAG_NONE,
			&indexResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&(geometry->indexBuffer))));
		// Copy the triangle data to the index buffer.
		UINT8* pIndexDataBegin;
		ThrowIfFailed(geometry->indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
		memcpy(pIndexDataBegin, indices, sizeof(indices));
		geometry->indexBuffer->Unmap(0, nullptr);

		// 初始化顶点/索引缓冲区视图
		geometry->vertexBufferView.BufferLocation = geometry->vertexBuffer->GetGPUVirtualAddress();
		geometry->vertexBufferView.StrideInBytes = geometry->vertexStride;
		geometry->vertexBufferView.SizeInBytes = geometry->vertexBufferSize;
		geometry->indexBufferView.BufferLocation = geometry->indexBuffer->GetGPUVirtualAddress();
		geometry->indexBufferView.Format = geometry->indexFormat;
		geometry->indexBufferView.SizeInBytes = geometry->indexBufferSize;

		m_geometry.push_back(geometry);
	}

	// 创建常量缓冲区
	{
		// m_objectCB = new UploadBuffer<SceneConstantBuffer>(m_device.Get(), 1, true);

		UINT constantBufferSize = D3D12Utils::CalcConstantBufferByteSize(sizeof(SceneConstantBuffer)); // CB size is required to be 256-byte aligned.

		//D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_objectCB->Resource()->GetGPUVirtualAddress();
		//// Offset to the ith object constant buffer in the buffer.
		//int boxCBufIndex = 0;
		//cbAddress += boxCBufIndex * objCBByteSize;

		CD3DX12_HEAP_PROPERTIES hpUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto constantResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);
		ThrowIfFailed(m_device->CreateCommittedResource(
			&hpUpload,
			D3D12_HEAP_FLAG_NONE,
			&constantResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constantBuffer)));

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = constantBufferSize;
		m_device->CreateConstantBufferView(
			&cbvDesc,
			m_cbvHeap->GetCPUDescriptorHandleForHeapStart());

		// Map and initialize the constant buffer. We don't unmap this until the app closes. Keeping things mapped for the lifetime of the resource is okay.
		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
		memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
	}

	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	{
		// 创建并初始化围栏
		ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_fence->SetName(L"m_fence");
		m_fenceValue = 1;

		// 创建用于帧同步的事件句柄
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));

		WaitForPreviousFrame();
	}
}

void D3D12FromIntroToGivingUp::OnUpdate() {
	const float rotateSpeed = 1;
	m_constantBufferData.rotation += rotateSpeed;
	// printf("%f\n", m_constantBufferData.rotation);
	memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
}

void D3D12FromIntroToGivingUp::OnRender() {
    PopulateCommandList(); // 填充命令列表
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	ThrowIfFailed(m_swapChain->Present(1, 0));
	WaitForPreviousFrame();
}

void D3D12FromIntroToGivingUp::OnDestroy() {
    WaitForPreviousFrame();
    CloseHandle(m_fenceEvent);
}

/*void D3D12FromIntroToGivingUp::OnResize() {
	// Flush before changing any resources
	WaitForPreviousFrame();

	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));
	for (int i = 0; i < frameCount; ++i)
		m_renderTargets[i].Reset();
	m_depthStencil.Reset();
	ThrowIfFailed(m_swapChain->ResizeBuffers(
		frameCount,
		m_width, m_height,
		m_backBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
	m_frameIndex = 0;
	if (m_height != 0)
		m_aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);

	// 将资源从初始状态转换为深度缓冲区
	CD3DX12_RESOURCE_BARRIER pBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_depthStencil.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_DEPTH_WRITE
	);
	m_commandList->ResourceBarrier(1, &pBarrier);

	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	WaitForPreviousFrame();

	// Update the viewport transform to cover the client area.
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = static_cast<float>(m_width);
	m_viewport.Height = static_cast<float>(m_height);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_scissorRect = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
}*/

void D3D12FromIntroToGivingUp::PopulateCommandList() {
	ThrowIfFailed(m_commandAllocator->Reset()); // 只有当相关的命令列表在GPU上完成执行时，才能重置分配器;应用程序应该使用围栏来确定GPU的执行进度
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get())); // 但是，当对特定命令列表调用ExecuteCommandList()时，该命令列表可以在任何时候重置，并且必须在重新记录之前重置

	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	ID3D12DescriptorHeap* ppHeaps[] = { m_cbvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	m_commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	// Indicate that the back buffer will be used as a render target.
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &barrier);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	//D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	//m_commandList->OMSetRenderTargets(1, &rtvHandle, TRUE, &dsvHandle);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, TRUE, nullptr);

	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	//m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(0, 1, &m_geometry[0]->vertexBufferView);
	m_commandList->IASetIndexBuffer(&m_geometry[0]->indexBufferView);
	m_commandList->DrawIndexedInstanced(4, 1, 0, 0, 0);

	// 指示后台缓冲区现在将用于呈现
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &barrier);

	ThrowIfFailed(m_commandList->Close());
}

void D3D12FromIntroToGivingUp::WaitForPreviousFrame() {
	// 在继续之前等待帧完成不是最佳实践
	// 这是为了简单而实现的代码,更高级的示例说明了如何使用围栏来有效地使用资源
	const UINT64 fence = m_fenceValue;
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
	m_fenceValue++;

	if (m_fence->GetCompletedValue() < fence) {
		ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}