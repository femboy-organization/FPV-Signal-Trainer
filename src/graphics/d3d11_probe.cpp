#include "graphics/d3d11_probe.h"

#include <d3d11.h>
#include <wrl/client.h>

namespace fpv::graphics {

std::wstring QueryD3D11Status() {
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    D3D_FEATURE_LEVEL featureLevel{};

    HRESULT result = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        device.GetAddressOf(),
        &featureLevel,
        context.GetAddressOf());

    bool usingWarp = false;
    if (FAILED(result)) {
        result = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            device.ReleaseAndGetAddressOf(),
            &featureLevel,
            context.ReleaseAndGetAddressOf());
        usingWarp = SUCCEEDED(result);
    }

    if (FAILED(result)) {
        return L"Direct3D 11: initialization failed.";
    }

    if (usingWarp) {
        return L"Direct3D 11: WARP software fallback (not suitable for final VR use).";
    }

    return L"Direct3D 11: hardware device ready.";
}

} // namespace fpv::graphics
