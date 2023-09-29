#include "pch.h"

namespace DX {

    inline void ThrowIfFailed(HRESULT hr) {
        if (FAILED(hr)) {
            // Set a breakpoint on this line to catch DirectX API errors
            throw std::exception();
        }
    }
}