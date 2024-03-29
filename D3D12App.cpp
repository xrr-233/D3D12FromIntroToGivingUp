﻿#include "D3D12App.h"

D3D12App::D3D12App(UINT width, UINT height, std::wstring name) :
	m_width(width),
	m_height(height),
	m_title(name) {
	m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

D3D12App::~D3D12App() {
}