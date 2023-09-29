#pragma once

#include "resource.h"
#include "D3D12App.h"

class D3D12FromIntroToGivingUp : D3D12App {
public:
    D3D12FromIntroToGivingUp(HINSTANCE hInstance);
    ~D3D12FromIntroToGivingUp();
    
private:
    UINT m_clientWidth = 640;
    UINT m_clientHeight = 480;
};