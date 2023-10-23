#pragma once

#include "DXHelper.h"
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

	// Accessors
	UINT GetWidth() const { return m_width; }
	UINT GetHeight() const { return m_height; }
	const WCHAR* GetTitle() const { return m_title.c_str(); }

protected:
	UINT m_width;
	UINT m_height;
	float m_aspectRatio;

private:
	std::wstring m_assetsPath;
	std::wstring m_title;
};