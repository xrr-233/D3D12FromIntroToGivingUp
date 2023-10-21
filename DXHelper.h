#include "pch.h"
#include "framework.h"

namespace DX {

    inline void ThrowIfFailed(HRESULT hr) {
        if (FAILED(hr)) {
            // Set a breakpoint on this line to catch DirectX API errors
            _com_error err(hr);
            wprintf(L"0x%x\n%s\n", err.Error(), err.ErrorMessage());
            throw std::exception();
        }
    }
}