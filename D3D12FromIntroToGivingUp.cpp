#include "D3D12FromIntroToGivingUp.h"

D3D12FromIntroToGivingUp::D3D12FromIntroToGivingUp(UINT width, UINT height, std::wstring name) {
    this->m_clientWidth = width;
    this->m_clientHeight = height;

    this->m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow) {
    D3D12FromIntroToGivingUp app(1280, 720, L"我的应用");
    Win32App* win32App = Win32App::GetInstance(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
    win32App->Run(&app);
}