#pragma once

#include "resource.h"
#include "D3D12App.h"

#define MAX_LOADSTRING 100

class Win32Application {
public:
	static Win32Application* GetInstance(HINSTANCE, HINSTANCE, LPWSTR, int);
	static Win32Application* GetInstance();

	int Run(D3D12App*);
	HWND GetHwnd() { return m_hwnd; }

private:
	Win32Application(HINSTANCE, HINSTANCE, LPWSTR, int);
	~Win32Application();
	static Win32Application* instance;
	HWND m_hwnd;

	// 全局变量:
	HINSTANCE	hInstance;							// 当前实例
	HINSTANCE	hPrevInstance;
	LPWSTR		lpCmdLine;
	int			nCmdShow;
	WCHAR		szTitle[MAX_LOADSTRING];			// 标题栏文本
	WCHAR		szWindowClass[MAX_LOADSTRING];		// 主窗口类名

	// 此代码模块中包含的函数的前向声明:
	ATOM                MyRegisterClass();
	BOOL                InitInstance();
	static LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
};