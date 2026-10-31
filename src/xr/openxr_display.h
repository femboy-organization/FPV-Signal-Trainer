#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#ifndef XR_USE_PLATFORM_WIN32
#define XR_USE_PLATFORM_WIN32
#endif
#ifndef XR_USE_GRAPHICS_API_D3D11
#define XR_USE_GRAPHICS_API_D3D11
#endif

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <string>
#include <vector>

namespace fpv::xr {

class OpenXrDisplay final {
public:
    OpenXrDisplay() = default;
    ~OpenXrDisplay();

    OpenXrDisplay(const OpenXrDisplay&) = delete;
    OpenXrDisplay& operator=(const OpenXrDisplay&) = delete;

    [[nodiscard]] bool Initialize();
    void PollEvents();
    void RenderFrame();
    void Shutdown() noexcept;

    [[nodiscard]] bool IsReady() const noexcept;
    [[nodiscard]] bool IsSessionRunning() const noexcept;
    [[nodiscard]] const std::wstring& Status() const noexcept;

private:
    [[nodiscard]] bool CreateInstance();
    [[nodiscard]] bool CreateSystem();
    [[nodiscard]] bool CreateGraphicsDevice();
    [[nodiscard]] bool CreateSession();
    [[nodiscard]] bool CreateViewSpace();
    [[nodiscard]] bool CreateSwapchain();
    [[nodiscard]] bool CreateTestPatternResources();
    [[nodiscard]] bool RenderTestPattern(std::uint32_t imageIndex);
    void HandleSessionState(XrSessionState state);
    void SetFailure(const wchar_t* operation, XrResult result);
    void ReleaseResources() noexcept;

    XrInstance instance_{XR_NULL_HANDLE};
    XrSystemId systemId_{XR_NULL_SYSTEM_ID};
    XrSession session_{XR_NULL_HANDLE};
    XrSpace viewSpace_{XR_NULL_HANDLE};
    XrSwapchain swapchain_{XR_NULL_HANDLE};

    std::vector<XrSwapchainImageD3D11KHR> swapchainImages_;
    std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> renderTargets_;

    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_;

    XrSessionState sessionState_{XR_SESSION_STATE_UNKNOWN};
    bool sessionRunning_{false};
    bool initialized_{false};

    std::uint32_t panelWidth_{1440};
    std::uint32_t panelHeight_{810};
    float panelWidthMeters_{1.35F};
    float panelHeightMeters_{0.759F};
    float panelDistanceMeters_{1.35F};

    std::wstring status_;
};

} // namespace fpv::xr
