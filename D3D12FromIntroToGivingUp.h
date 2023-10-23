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

private:
	// 全局设置
	static const int frameCount = 2;
	bool m_4xMsaaEnabled = FALSE;
	UINT m_4xMsaaQuality = 1;
	DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	struct Vertex {
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

	// 管线对象
	ComPtr<ID3D12Device>			m_device;
	ComPtr<ID3D12CommandQueue>		m_commandQueue;
	ComPtr<ID3D12CommandAllocator>	m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<IDXGISwapChain3>			m_swapChain;
	ComPtr<ID3D12DescriptorHeap>	m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap>	m_dsvHeap;
	ComPtr<ID3D12Resource>			m_renderTargets[frameCount];
	ComPtr<ID3D12Resource>			m_depthStencil;
	UINT			m_rtvDescriptorSize;
	D3D12_VIEWPORT	m_viewport;
	D3D12_RECT		m_scissorRect;

	ComPtr<ID3D12RootSignature>		m_rootSignature;
	ComPtr<ID3D12PipelineState>		m_pipelineState;

	// 应用对象
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	// 同步对象
	UINT m_frameIndex;
	ComPtr<ID3D12Fence> m_fence;
	HANDLE m_fenceEvent;
	UINT64 m_fenceValue;

	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList();
	void WaitForPreviousFrame();
};