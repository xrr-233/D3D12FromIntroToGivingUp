#pragma once

#include "D3D12Utils.h"
#include "D3D12MathHelper.h"
#include "Win32App.h"

using namespace DirectX;
using namespace Microsoft::WRL;

class D3D12App {
public:
	D3D12App(UINT width, UINT height, std::wstring name);
	virtual ~D3D12App();

	virtual void OnInit() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnRender() = 0;
	virtual void OnDestroy() = 0;
	virtual void OnResize() = 0;

	// Accessors
	UINT GetWidth() const { return m_width; }
	UINT GetHeight() const { return m_height; }
	const WCHAR* GetTitle() const { return m_title.c_str(); }
	void SetWidth(UINT width) const { width = m_width; }
	void SetHeight(UINT height) const { height = m_height; }

protected:
	UINT	m_width;
	UINT	m_height;
	float	m_aspectRatio;

	// 全局设置
	static const int frameCount = 2;
	bool m_4xMsaaEnabled = FALSE;
	UINT m_4xMsaaQuality = 0;
	DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

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

	void BuildCommandObjects();
	void BuildSwapChain();
	void BuildRtvHeap();
	void BuildDsvHeap();
	void BuildCbvHeap();

private:
	std::wstring m_assetsPath;
	std::wstring m_title;
};