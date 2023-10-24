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

	// 管线对象
	ComPtr<ID3D12Resource>			m_renderTargets[frameCount];
	ComPtr<ID3D12Resource>			m_depthStencil;
	UploadBuffer<ObjectConstants>*	m_objectCB = nullptr;

	ComPtr<ID3D12RootSignature>		m_rootSignature;
	ComPtr<ID3DBlob>				vertexShader;
	ComPtr<ID3DBlob>				pixelShader;
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
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

	// detailed steps
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildGeometry();
	void BuildPSO();

	void BuildConstantBuffers();
};