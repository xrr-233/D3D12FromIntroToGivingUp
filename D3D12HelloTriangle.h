#include "pch.h"

using namespace DirectX;
using namespace Microsoft::WRL;

class D3D12HelloTriangle {
public:
	D3D12HelloTriangle(UINT width, UINT height, std::wstring name);

	void OnInit();
	void OnUpdate();
	void OnRender();
	void OnDestroy();

private:
	// 全局设置
	static const int frameCount = 2;
	UINT m_clientWidth = 800;
	UINT m_clientHeight = 600;
	bool m_4xMsaaEnabled = true;
	UINT m_4xMsaaQuality = 1;

	struct Vertex {
		XMFLOAT3 position;
		XMFLOAT4 color;
	};
	
	// 管线对象
	ComPtr<ID3D12Device>			m_device;
	ComPtr<ID3D12CommandQueue>		m_commandQueue;
	ComPtr<ID3D12CommandAllocator>	m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<IDXGISwapChain>			m_swapChain; // IDXGISwapChain3?
	ComPtr<ID3D12DescriptorHeap>	m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap>	m_dsvHeap;
	ComPtr<ID3D12Resource>			m_renderTargets[frameCount];
	ComPtr<ID3D12Resource>			m_depthStencil;
	UINT			m_rtvDescriptorSize;
	D3D12_VIEWPORT	m_viewport;
	D3D12_RECT		m_scissorRect;

	// 同步对象
	UINT m_frameIndex;

	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList();
	void WaitForPreviousFrame();
};