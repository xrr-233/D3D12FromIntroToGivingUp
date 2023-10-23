#pragma once

#include "pch.h"
#include "framework.h"

inline void ThrowIfFailed(HRESULT hr) {
    // https://learn.microsoft.com/zh-cn/windows/win32/com/com-error-codes-10
    if (FAILED(hr)) {
        _com_error err(hr);
        wprintf(L"0x%x\n%s\n", err.Error(), err.ErrorMessage());
        throw std::exception();
    }
}