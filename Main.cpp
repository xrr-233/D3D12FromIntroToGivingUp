#include "D3D12FromIntroToGivingUp.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow) {
    // Create a separate console window to display output to stdout
    AllocConsole();
    setlocale(LC_ALL, "CHS");
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);

    D3D12FromIntroToGivingUp* app = new D3D12FromIntroToGivingUp(1280, 720, L"D3D12：从入门到放弃");
    Win32App* win32App = Win32App::GetInstance(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
    win32App->Run(app);
}