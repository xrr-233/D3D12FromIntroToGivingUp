﻿#pragma once

#include "resource.h"
#include "D3D12App.h"

#define MAX_LOADSTRING 100

class D3D12App;

class Win32App {
public:
	static Win32App* GetInstance(HINSTANCE, HINSTANCE, LPWSTR, int);
	static Win32App* GetInstance();

	D3D12App* app;
	int Run(D3D12App*);
	static HWND GetHwnd() { return instance->m_hwnd; }

private:
	Win32App(HINSTANCE, HINSTANCE, LPWSTR, int);
	static Win32App* instance;
	HWND m_hwnd;

	// 全局变量:
	HINSTANCE	hInstance;							// 当前实例
	HINSTANCE	hPrevInstance;
	LPWSTR		lpCmdLine;
	int			nCmdShow;
	WCHAR		szWindowClass[MAX_LOADSTRING];		// 主窗口类名

	// 此代码模块中包含的函数的前向声明:
	ATOM		MyRegisterClass();
	void		MyShowWindow();
	static LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
};