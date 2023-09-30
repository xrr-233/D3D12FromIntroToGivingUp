#pragma once

#include "resource.h"
#include "D3D12App.h"
#include "Win32Application.h"

class D3D12FromIntroToGivingUp : public D3D12App {
public:
    D3D12FromIntroToGivingUp(UINT width, UINT height, std::wstring name);
};