#include "pch.h"

using namespace DirectX;
using namespace Microsoft::WRL;

class D3D12App {
public:
	D3D12App(UINT width, UINT height, std::wstring name);
	~D3D12App();

	void OnInit();
	void OnUpdate();
	void OnRender();
	void OnDestroy();

private:
	// 全局设置
	static const int frameCount = 2;
	UINT m_clientWidth;
	UINT m_clientHeight;
	float m_aspectRatio;
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