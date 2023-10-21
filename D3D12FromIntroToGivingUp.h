#pragma once

#include "resource.h"
#include "framework.h"
#include "D3D12App.h"
#include "Win32App.h"

class D3D12FromIntroToGivingUp : public D3D12App {
public:
    D3D12FromIntroToGivingUp(UINT width, UINT height, std::wstring name);
};