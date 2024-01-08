#pragma once

#include "resource.h"
#include "framework.h"
#include "D3D12App.h"
#include "Win32App.h"

class D3D12FromIntroToGivingUp : public D3D12App {
public:
    D3D12FromIntroToGivingUp(UINT width, UINT height, std::wstring name);

	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();

	virtual void OnResize();

private:
	struct Vertex {
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

	struct ObjectConstants
	{
		XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
	};

	// 全局设置
	static const int frameCount = 2;
	// bool m_4xMsaaEnabled = FALSE;
	// UINT m_4xMsaaQuality = 0;
	DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	// DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	ComPtr<IDXGIFactory4>			m_dxgiFactory;
	ComPtr<ID3D12Device>			m_device;
	ComPtr<ID3D12CommandQueue>		m_commandQueue;
	ComPtr<ID3D12CommandAllocator>	m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<IDXGISwapChain3>			m_swapChain;
	ComPtr<ID3D12DescriptorHeap>	m_rtvHeap, m_dsvHeap, m_cbvHeap;
	D3D12_VIEWPORT	m_viewport;
	D3D12_RECT		m_scissorRect;

	UINT	m_rtvDescriptorSize;
	UINT	m_dsvDescriptorSize;
	UINT	m_cbvSrvUavDescriptorSize;

	// 管线对象
	ComPtr<ID3D12Resource>			m_renderTargets[frameCount];
	ComPtr<ID3D12Resource>			m_depthStencil;
	UploadBuffer<ObjectConstants>*	m_objectCB = nullptr;

	ComPtr<ID3D12RootSignature>		m_rootSignature;
	ComPtr<ID3DBlob>				vertexShader;
	ComPtr<ID3DBlob>				pixelShader;
	ComPtr<ID3D12PipelineState>		m_pipelineState;

	// 应用对象
	std::vector<MeshGeometry*>		m_geometry;

	// 同步对象
	UINT				m_frameIndex;
	ComPtr<ID3D12Fence>	m_fence;
	HANDLE				m_fenceEvent;
	UINT64				m_fenceValue;

	// main steps
	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList();
	void WaitForPreviousFrame(); // FlushCommandQueue
};