#include "D3D12FromIntroToGivingUp.h"

D3D12FromIntroToGivingUp::D3D12FromIntroToGivingUp(UINT width, UINT height, std::wstring name) :
    D3D12App(width, height, name) {
}

void D3D12FromIntroToGivingUp::OnInit() {
    LoadPipeline();
    LoadAssets();
}

void D3D12FromIntroToGivingUp::OnUpdate() {
}

void D3D12FromIntroToGivingUp::OnRender() {
    // 将渲染场景所需的所有命令记录到命令列表中
    PopulateCommandList();
}

void D3D12FromIntroToGivingUp::OnDestroy() {
    WaitForPreviousFrame();
    CloseHandle(m_fenceEvent);
}

void D3D12FromIntroToGivingUp::OnResize() {
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

	// 创建帧资源
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < frameCount; i++) {
		ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
		m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, m_rtvDescriptorSize);
	}

	D3D12_RESOURCE_DESC depthStencilDesc;
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
	m_device->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
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
}

// https://learn.microsoft.com/en-us/windows/win32/direct3d12/creating-a-basic-direct3d-12-component
// main steps
void D3D12FromIntroToGivingUp::LoadPipeline() {
	// 启用调试层
#if defined(DEBUG) || defined(_DEBUG)
	ComPtr<ID3D12Debug> debugController;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
	debugController->EnableDebugLayer();
#endif

	// 创建设备
	ThrowIfFailed(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_dxgiFactory)));
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

	// 创建并初始化围栏
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fence->SetName(L"m_fence");
	m_fenceValue = 1;

	// 检测对4X MSAA质量级别的支持
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
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
	assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

	BuildCommandObjects();
	BuildSwapChain();
	BuildRtvHeap();
	BuildDsvHeap();
	BuildCbvHeap();

	BuildConstantBuffers();

	OnResize();
}

void D3D12FromIntroToGivingUp::LoadAssets() {
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildGeometry();
	BuildPSO();

	// 创建用于帧同步的事件句柄
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));

	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	WaitForPreviousFrame();
}

void D3D12FromIntroToGivingUp::PopulateCommandList() {
	// 重置命令分配器和命令列表
	ThrowIfFailed(m_commandAllocator->Reset()); // 只有当相关的命令列表在GPU上完成执行时，才能重置命令列表分配器;应用程序应该使用围栏来确定GPU的执行进度
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get())); // 但是，当对特定命令列表调用ExecuteCommandList()时，该命令列表可以在任何时候重置，并且必须在重新记录之前重置

	// 设置根签名、视口和剪刀矩形
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	// 指示后台缓冲区将用作渲染目标
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &barrier);

	// 记录一下命令
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, TRUE, &dsvHandle);

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(0, 1, &m_geometry[0]->vertexBufferView);
	m_commandList->IASetIndexBuffer(&m_geometry[0]->indexBufferView);

	m_commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());

	m_commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

	// 指示后台缓冲区现在将用于呈现
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &barrier);

	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	ThrowIfFailed(m_swapChain->Present(1, 0));
	WaitForPreviousFrame();
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

// detailed steps
void D3D12FromIntroToGivingUp::BuildRootSignature() {
	// Shader programs typically require resources as input (constant buffers,
	// textures, samplers).  The root signature defines the resources the shader
	// programs expect.  If we think of the shader programs as a function, and
	// the input resources as function parameters, then the root signature can be
	// thought of as defining the function signature.  

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];
	// Create a single descriptor table of CBVs.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(1, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	m_rootSignature->SetName(L"m_rootSignature");
}

void D3D12FromIntroToGivingUp::BuildShadersAndInputLayout() {
#if defined(DEBUG) || defined(_DEBUG)
	// 使用图形调试工具启用更好的着色器调试
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
	ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

	// 创建顶点输入布局
	m_inputLayout = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};
}

void D3D12FromIntroToGivingUp::BuildGeometry() {
	// 创建顶点缓冲区
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

	ThrowIfFailed(D3DCreateBlob(geometry->vertexBufferSize, &geometry->vertexBufferCPU));
	CopyMemory(geometry->vertexBufferCPU->GetBufferPointer(), vertices, geometry->vertexBufferSize);
	ThrowIfFailed(D3DCreateBlob(geometry->indexBufferSize, &geometry->indexBufferCPU));
	CopyMemory(geometry->indexBufferCPU->GetBufferPointer(), indices, geometry->indexBufferSize);
	geometry->vertexBufferGPU = D3D12Utils::CreateDefaultBuffer(m_device.Get(),
		m_commandList.Get(), vertices, geometry->vertexBufferSize, geometry->vertexBufferUploader);
	geometry->indexBufferGPU = D3D12Utils::CreateDefaultBuffer(m_device.Get(),
		m_commandList.Get(), indices, geometry->indexBufferSize, geometry->indexBufferUploader);

	// 初始化顶点缓冲区视图
	geometry->vertexBufferView.BufferLocation = geometry->vertexBufferGPU->GetGPUVirtualAddress();
	geometry->vertexBufferView.StrideInBytes = sizeof(Vertex);
	geometry->vertexBufferView.SizeInBytes = geometry->vertexBufferSize;
	geometry->indexBufferView.BufferLocation = geometry->indexBufferGPU->GetGPUVirtualAddress();
	geometry->indexBufferView.Format = geometry->indexFormat;
	geometry->indexBufferView.SizeInBytes = geometry->indexBufferSize;

	m_geometry.push_back(geometry);
}

void D3D12FromIntroToGivingUp::BuildPSO() {
	// 描述与创建图形管道状态对象
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	//psoDesc.DepthStencilState.DepthEnable = FALSE;
	//psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = m_backBufferFormat;
	psoDesc.SampleDesc.Count = m_4xMsaaEnabled ? 4 : 1;
	psoDesc.SampleDesc.Quality = m_4xMsaaEnabled ? (m_4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = m_depthStencilFormat;
	ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}


void D3D12FromIntroToGivingUp::BuildConstantBuffers()
{
	m_objectCB = new UploadBuffer<ObjectConstants>(m_device.Get(), 1, true);

	UINT objCBByteSize = D3D12Utils::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_objectCB->Resource()->GetGPUVirtualAddress();
	// Offset to the ith object constant buffer in the buffer.
	int boxCBufIndex = 0;
	cbAddress += boxCBufIndex * objCBByteSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = D3D12Utils::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	m_device->CreateConstantBufferView(
		&cbvDesc,
		m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
}