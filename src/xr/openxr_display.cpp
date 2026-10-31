#include "xr/openxr_display.h"

#include <d3dcompiler.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <string>

namespace fpv::xr {
namespace {

constexpr char kD3D11Extension[] = XR_KHR_D3D11_ENABLE_EXTENSION_NAME;

struct Vertex final {
    float x;
    float y;
    float u;
    float v;
};

constexpr std::array<Vertex, 3> kFullscreenTriangle{{
    {-1.0F, -1.0F, 0.0F, 1.0F},
    {-1.0F,  3.0F, 0.0F, -1.0F},
    { 3.0F, -1.0F, 2.0F, 1.0F}
}};

constexpr char kShaderSource[] = R"(
struct VSInput { float2 position : POSITION; float2 texcoord : TEXCOORD0; };
struct PSInput { float4 position : SV_POSITION; float2 texcoord : TEXCOORD0; };

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.position = float4(input.position, 0.0, 1.0);
    output.texcoord = input.texcoord;
    return output;
}

float3 ColorBar(float x)
{
    if (x < 1.0 / 7.0) return float3(1.0, 1.0, 1.0);
    if (x < 2.0 / 7.0) return float3(1.0, 1.0, 0.0);
    if (x < 3.0 / 7.0) return float3(0.0, 1.0, 1.0);
    if (x < 4.0 / 7.0) return float3(0.0, 1.0, 0.0);
    if (x < 5.0 / 7.0) return float3(1.0, 0.0, 1.0);
    if (x < 6.0 / 7.0) return float3(1.0, 0.0, 0.0);
    return float3(0.0, 0.0, 1.0);
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float2 uv = saturate(input.texcoord);
    float3 color = lerp(float3(0.015, 0.035, 0.065),
                        float3(0.025, 0.19, 0.24), uv.y);

    float gridX = step(0.985, frac(uv.x * 24.0));
    float gridY = step(0.985, frac(uv.y * 14.0));
    color = lerp(color, float3(0.08, 0.22, 0.25),
                 max(gridX, gridY) * 0.45);

    float centerX = 1.0 - step(0.0025, abs(uv.x - 0.5));
    float centerY = 1.0 - step(0.0035, abs(uv.y - 0.5));
    color = lerp(color, float3(0.75, 0.95, 0.90), max(centerX, centerY));

    float radius = length((uv - float2(0.5, 0.5)) * float2(1.0, 1.777));
    float ring = 1.0 - step(0.0025, abs(radius - 0.22));
    color = lerp(color, float3(0.15, 0.95, 0.70), ring);

    if (uv.y > 0.82) color = ColorBar(uv.x);
    if (uv.y > 0.80 && uv.y < 0.82) color = float3(0.015, 0.02, 0.025);
    return float4(color, 1.0);
}
)";

const wchar_t* ResultName(XrResult result) {
    switch (result) {
    case XR_SUCCESS: return L"XR_SUCCESS";
    case XR_ERROR_RUNTIME_UNAVAILABLE: return L"XR_ERROR_RUNTIME_UNAVAILABLE";
    case XR_ERROR_API_VERSION_UNSUPPORTED: return L"XR_ERROR_API_VERSION_UNSUPPORTED";
    case XR_ERROR_EXTENSION_NOT_PRESENT: return L"XR_ERROR_EXTENSION_NOT_PRESENT";
    case XR_ERROR_EXTENSION_DEPENDENCY_NOT_ENABLED: return L"XR_ERROR_EXTENSION_DEPENDENCY_NOT_ENABLED";
    case XR_ERROR_INITIALIZATION_FAILED: return L"XR_ERROR_INITIALIZATION_FAILED";
    case XR_ERROR_VALIDATION_FAILURE: return L"XR_ERROR_VALIDATION_FAILURE";
    case XR_ERROR_RUNTIME_FAILURE: return L"XR_ERROR_RUNTIME_FAILURE";
    case XR_ERROR_GRAPHICS_DEVICE_INVALID: return L"XR_ERROR_GRAPHICS_DEVICE_INVALID";
    case XR_ERROR_FORM_FACTOR_UNAVAILABLE: return L"XR_ERROR_FORM_FACTOR_UNAVAILABLE";
    case XR_ERROR_INSTANCE_LOST: return L"XR_ERROR_INSTANCE_LOST";
    case XR_ERROR_SESSION_LOST: return L"XR_ERROR_SESSION_LOST";
    default: return L"OpenXR error";
    }
}

std::wstring ResultText(const wchar_t* operation, XrResult result) {
    std::wstring text(operation);
    text += L": ";
    text += ResultName(result);
    text += L" (";
    text += std::to_wstring(static_cast<std::int32_t>(result));
    text += L")";
    return text;
}

bool HasExtension(const std::vector<XrExtensionProperties>& properties, const char* name) {
    return std::any_of(properties.begin(), properties.end(),
        [name](const XrExtensionProperties& property) {
            return std::strcmp(property.extensionName, name) == 0;
        });
}

} // namespace

OpenXrDisplay::~OpenXrDisplay() {
    Shutdown();
}

bool OpenXrDisplay::Initialize() {
    if (initialized_) return true;

    status_ = L"Initializing OpenXR...";
    if (!CreateInstance() ||
        !CreateSystem() ||
        !CreateGraphicsDevice() ||
        !CreateSession() ||
        !CreateViewSpace() ||
        !CreateSwapchain() ||
        !CreateTestPatternResources()) {
        ReleaseResources();
        return false;
    }

    initialized_ = true;
    status_ = L"OpenXR initialized. Waiting for headset session READY event.";
    return true;
}

bool OpenXrDisplay::CreateInstance() {
    std::uint32_t extensionCount = 0;
    XrResult result = xrEnumerateInstanceExtensionProperties(
        nullptr, 0, &extensionCount, nullptr);
    if (XR_FAILED(result)) {
        SetFailure(L"xrEnumerateInstanceExtensionProperties", result);
        return false;
    }

    std::vector<XrExtensionProperties> extensions(extensionCount);
    for (auto& extension : extensions) {
        extension.type = XR_TYPE_EXTENSION_PROPERTIES;
        extension.next = nullptr;
    }

    if (extensionCount > 0) {
        result = xrEnumerateInstanceExtensionProperties(
            nullptr, extensionCount, &extensionCount, extensions.data());
        if (XR_FAILED(result)) {
            SetFailure(L"xrEnumerateInstanceExtensionProperties(data)", result);
            return false;
        }
    }

    if (!HasExtension(extensions, kD3D11Extension)) {
        status_ = L"OpenXR runtime does not advertise XR_KHR_D3D11_enable.";
        return false;
    }

    XrInstanceCreateInfo createInfo{XR_TYPE_INSTANCE_CREATE_INFO};
    createInfo.applicationInfo.apiVersion = XR_MAKE_VERSION(1, 0, 0);
    createInfo.applicationInfo.applicationVersion = 1;
    createInfo.applicationInfo.engineVersion = 1;
    strcpy_s(createInfo.applicationInfo.applicationName,
             XR_MAX_APPLICATION_NAME_SIZE, "FPV Signal Trainer");
    strcpy_s(createInfo.applicationInfo.engineName,
             XR_MAX_ENGINE_NAME_SIZE, "Native C++ / Direct3D 11");

    const char* enabledExtensions[] = {kD3D11Extension};
    createInfo.enabledExtensionCount = 1;
    createInfo.enabledExtensionNames = enabledExtensions;

    result = xrCreateInstance(&createInfo, &instance_);
    if (XR_FAILED(result)) {
        SetFailure(L"xrCreateInstance", result);
        return false;
    }

    XrInstanceProperties properties{XR_TYPE_INSTANCE_PROPERTIES};
    result = xrGetInstanceProperties(instance_, &properties);
    if (XR_FAILED(result)) {
        SetFailure(L"xrGetInstanceProperties", result);
        return false;
    }

    status_ = L"OpenXR runtime: ";
    for (const char* character = properties.runtimeName; *character != '\0'; ++character) {
        status_.push_back(static_cast<unsigned char>(*character));
    }
    status_ += L". Creating VR session...";
    return true;
}

bool OpenXrDisplay::CreateSystem() {
    XrSystemGetInfo getInfo{XR_TYPE_SYSTEM_GET_INFO};
    getInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

    const XrResult result = xrGetSystem(instance_, &getInfo, &systemId_);
    if (XR_FAILED(result)) {
        SetFailure(L"xrGetSystem(HMD)", result);
        return false;
    }
    return true;
}

bool OpenXrDisplay::CreateGraphicsDevice() {
    PFN_xrVoidFunction function = nullptr;
    XrResult result = xrGetInstanceProcAddr(
        instance_, "xrGetD3D11GraphicsRequirementsKHR", &function);
    if (XR_FAILED(result) || function == nullptr) {
        SetFailure(L"Load xrGetD3D11GraphicsRequirementsKHR", result);
        return false;
    }

    const auto getRequirements =
        reinterpret_cast<PFN_xrGetD3D11GraphicsRequirementsKHR>(function);

    XrGraphicsRequirementsD3D11KHR requirements{
        XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR
    };
    result = getRequirements(instance_, systemId_, &requirements);
    if (XR_FAILED(result)) {
        SetFailure(L"xrGetD3D11GraphicsRequirementsKHR", result);
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIFactory1> factory;
    const HRESULT factoryResult = CreateDXGIFactory1(
        __uuidof(IDXGIFactory1),
        reinterpret_cast<void**>(factory.GetAddressOf()));
    if (FAILED(factoryResult)) {
        status_ = L"CreateDXGIFactory1 failed; cannot select the OpenXR graphics adapter.";
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIAdapter1> selectedAdapter;
    for (UINT index = 0;; ++index) {
        Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
        const HRESULT enumResult = factory->EnumAdapters1(index, adapter.GetAddressOf());
        if (enumResult == DXGI_ERROR_NOT_FOUND) break;
        if (FAILED(enumResult)) continue;

        DXGI_ADAPTER_DESC1 description{};
        if (SUCCEEDED(adapter->GetDesc1(&description)) &&
            description.AdapterLuid.LowPart == requirements.adapterLuid.LowPart &&
            description.AdapterLuid.HighPart == requirements.adapterLuid.HighPart) {
            selectedAdapter = adapter;
            break;
        }
    }

    if (!selectedAdapter) {
        status_ = L"OpenXR-required DXGI adapter LUID was not found.";
        return false;
    }

    constexpr UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    constexpr std::array<D3D_FEATURE_LEVEL, 4> featureLevels{{
        D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0
    }};

    D3D_FEATURE_LEVEL selectedLevel{};
    HRESULT deviceResult = D3D11CreateDevice(
        selectedAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, flags,
        featureLevels.data(), static_cast<UINT>(featureLevels.size()),
        D3D11_SDK_VERSION, device_.GetAddressOf(), &selectedLevel,
        context_.GetAddressOf());

    if (deviceResult == E_INVALIDARG) {
        constexpr std::array<D3D_FEATURE_LEVEL, 3> fallbackLevels{{
            D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        }};
        deviceResult = D3D11CreateDevice(
            selectedAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, flags,
            fallbackLevels.data(), static_cast<UINT>(fallbackLevels.size()),
            D3D11_SDK_VERSION, device_.ReleaseAndGetAddressOf(), &selectedLevel,
            context_.ReleaseAndGetAddressOf());
    }

    if (FAILED(deviceResult)) {
        status_ = L"D3D11 device creation failed on the adapter required by OpenXR.";
        return false;
    }

    if (selectedLevel < requirements.minFeatureLevel) {
        status_ = L"D3D11 adapter does not meet the minimum feature level required by OpenXR.";
        return false;
    }

    return true;
}

bool OpenXrDisplay::CreateSession() {
    XrGraphicsBindingD3D11KHR binding{XR_TYPE_GRAPHICS_BINDING_D3D11_KHR};
    binding.device = device_.Get();

    XrSessionCreateInfo createInfo{XR_TYPE_SESSION_CREATE_INFO};
    createInfo.next = &binding;
    createInfo.systemId = systemId_;

    const XrResult result = xrCreateSession(instance_, &createInfo, &session_);
    if (XR_FAILED(result)) {
        SetFailure(L"xrCreateSession", result);
        return false;
    }
    return true;
}

bool OpenXrDisplay::CreateViewSpace() {
    XrReferenceSpaceCreateInfo createInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
    createInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
    createInfo.poseInReferenceSpace.orientation.w = 1.0F;

    const XrResult result = xrCreateReferenceSpace(session_, &createInfo, &viewSpace_);
    if (XR_FAILED(result)) {
        SetFailure(L"xrCreateReferenceSpace(VIEW)", result);
        return false;
    }
    return true;
}

bool OpenXrDisplay::CreateSwapchain() {
    std::uint32_t formatCount = 0;
    XrResult result = xrEnumerateSwapchainFormats(session_, 0, &formatCount, nullptr);
    if (XR_FAILED(result)) {
        SetFailure(L"xrEnumerateSwapchainFormats", result);
        return false;
    }

    std::vector<std::int64_t> formats(formatCount);
    result = xrEnumerateSwapchainFormats(
        session_, formatCount, &formatCount, formats.data());
    if (XR_FAILED(result)) {
        SetFailure(L"xrEnumerateSwapchainFormats(data)", result);
        return false;
    }

    std::int64_t selectedFormat = 0;
    constexpr std::array<std::int64_t, 2> preferredFormats{{
        DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM
    }};
    for (const auto preferred : preferredFormats) {
        if (std::find(formats.begin(), formats.end(), preferred) != formats.end()) {
            selectedFormat = preferred;
            break;
        }
    }

    if (selectedFormat == 0) {
        status_ = L"Runtime exposes no supported RGBA/BGRA OpenXR swapchain format.";
        return false;
    }

    XrSwapchainCreateInfo createInfo{XR_TYPE_SWAPCHAIN_CREATE_INFO};
    createInfo.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.format = selectedFormat;
    createInfo.sampleCount = 1;
    createInfo.width = panelWidth_;
    createInfo.height = panelHeight_;
    createInfo.faceCount = 1;
    createInfo.arraySize = 1;
    createInfo.mipCount = 1;

    result = xrCreateSwapchain(session_, &createInfo, &swapchain_);
    if (XR_FAILED(result)) {
        SetFailure(L"xrCreateSwapchain", result);
        return false;
    }

    std::uint32_t imageCount = 0;
    result = xrEnumerateSwapchainImages(swapchain_, 0, &imageCount, nullptr);
    if (XR_FAILED(result) || imageCount == 0) {
        SetFailure(L"xrEnumerateSwapchainImages(count)", result);
        return false;
    }

    swapchainImages_.resize(imageCount);
    for (auto& image : swapchainImages_) {
        image.type = XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR;
        image.next = nullptr;
        image.texture = nullptr;
    }

    result = xrEnumerateSwapchainImages(
        swapchain_, imageCount, &imageCount,
        reinterpret_cast<XrSwapchainImageBaseHeader*>(swapchainImages_.data()));
    if (XR_FAILED(result)) {
        SetFailure(L"xrEnumerateSwapchainImages(data)", result);
        return false;
    }

    renderTargets_.resize(imageCount);
    for (std::uint32_t index = 0; index < imageCount; ++index) {
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
        rtvDesc.Format = static_cast<DXGI_FORMAT>(selectedFormat);
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;

        const HRESULT rtvResult = device_->CreateRenderTargetView(
            swapchainImages_[index].texture,
            &rtvDesc,
            renderTargets_[index].GetAddressOf());
        if (FAILED(rtvResult)) {
            status_ = L"Could not create a D3D11 render target for an OpenXR swapchain image.";
            return false;
        }
    }

    return true;
}

bool OpenXrDisplay::CreateTestPatternResources() {
    Microsoft::WRL::ComPtr<ID3DBlob> vertexBytecode;
    Microsoft::WRL::ComPtr<ID3DBlob> pixelBytecode;
    Microsoft::WRL::ComPtr<ID3DBlob> errors;

    HRESULT result = D3DCompile(
        kShaderSource, sizeof(kShaderSource) - 1, "FPVSignalTrainerTestPattern",
        nullptr, nullptr, "VSMain", "vs_4_0",
        D3DCOMPILE_ENABLE_STRICTNESS, 0,
        vertexBytecode.GetAddressOf(), errors.GetAddressOf());
    if (FAILED(result)) {
        status_ = L"D3DCompile failed for the test-pattern vertex shader.";
        return false;
    }

    errors.Reset();
    result = D3DCompile(
        kShaderSource, sizeof(kShaderSource) - 1, "FPVSignalTrainerTestPattern",
        nullptr, nullptr, "PSMain", "ps_4_0",
        D3DCOMPILE_ENABLE_STRICTNESS, 0,
        pixelBytecode.GetAddressOf(), errors.GetAddressOf());
    if (FAILED(result)) {
        status_ = L"D3DCompile failed for the test-pattern pixel shader.";
        return false;
    }

    result = device_->CreateVertexShader(
        vertexBytecode->GetBufferPointer(), vertexBytecode->GetBufferSize(),
        nullptr, vertexShader_.GetAddressOf());
    if (FAILED(result)) {
        status_ = L"Could not create the D3D11 vertex shader.";
        return false;
    }

    result = device_->CreatePixelShader(
        pixelBytecode->GetBufferPointer(), pixelBytecode->GetBufferSize(),
        nullptr, pixelShader_.GetAddressOf());
    if (FAILED(result)) {
        status_ = L"Could not create the D3D11 pixel shader.";
        return false;
    }

    constexpr std::array<D3D11_INPUT_ELEMENT_DESC, 2> inputElements{{
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0}
    }};

    result = device_->CreateInputLayout(
        inputElements.data(), static_cast<UINT>(inputElements.size()),
        vertexBytecode->GetBufferPointer(), vertexBytecode->GetBufferSize(),
        inputLayout_.GetAddressOf());
    if (FAILED(result)) {
        status_ = L"Could not create the D3D11 input layout.";
        return false;
    }

    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(kFullscreenTriangle));
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initialData{};
    initialData.pSysMem = kFullscreenTriangle.data();
    result = device_->CreateBuffer(&bufferDesc, &initialData, vertexBuffer_.GetAddressOf());
    if (FAILED(result)) {
        status_ = L"Could not create the D3D11 test-pattern vertex buffer.";
        return false;
    }

    D3D11_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.DepthClipEnable = TRUE;
    result = device_->CreateRasterizerState(
        &rasterizerDesc, rasterizerState_.GetAddressOf());
    if (FAILED(result)) {
        status_ = L"Could not create the D3D11 rasterizer state.";
        return false;
    }

    return true;
}

bool OpenXrDisplay::RenderTestPattern(std::uint32_t imageIndex) {
    if (imageIndex >= renderTargets_.size()) return false;

    ID3D11RenderTargetView* target = renderTargets_[imageIndex].Get();
    context_->OMSetRenderTargets(1, &target, nullptr);

    D3D11_VIEWPORT viewport{};
    viewport.Width = static_cast<float>(panelWidth_);
    viewport.Height = static_cast<float>(panelHeight_);
    viewport.MinDepth = 0.0F;
    viewport.MaxDepth = 1.0F;
    context_->RSSetViewports(1, &viewport);
    context_->RSSetState(rasterizerState_.Get());

    constexpr float clearColor[4]{0.015F, 0.035F, 0.065F, 1.0F};
    context_->ClearRenderTargetView(target, clearColor);

    const UINT stride = sizeof(Vertex);
    constexpr UINT offset = 0;
    ID3D11Buffer* vertexBuffer = vertexBuffer_.Get();

    context_->IASetInputLayout(inputLayout_.Get());
    context_->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context_->VSSetShader(vertexShader_.Get(), nullptr, 0);
    context_->PSSetShader(pixelShader_.Get(), nullptr, 0);
    context_->Draw(3, 0);
    context_->OMSetRenderTargets(0, nullptr, nullptr);
    return true;
}

void OpenXrDisplay::PollEvents() {
    if (instance_ == XR_NULL_HANDLE) return;

    XrEventDataBuffer eventBuffer{XR_TYPE_EVENT_DATA_BUFFER};
    while (xrPollEvent(instance_, &eventBuffer) == XR_SUCCESS) {
        if (eventBuffer.type == XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED) {
            const auto* changed =
                reinterpret_cast<const XrEventDataSessionStateChanged*>(&eventBuffer);
            if (changed->session == session_) {
                HandleSessionState(changed->state);
            }
        } else if (eventBuffer.type == XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING) {
            status_ = L"OpenXR instance loss pending. Restart Quest Link and this application.";
            sessionRunning_ = false;
        }
        eventBuffer = {XR_TYPE_EVENT_DATA_BUFFER};
    }
}

void OpenXrDisplay::HandleSessionState(XrSessionState state) {
    sessionState_ = state;

    if (state == XR_SESSION_STATE_READY && !sessionRunning_) {
        XrSessionBeginInfo beginInfo{XR_TYPE_SESSION_BEGIN_INFO};
        beginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
        const XrResult result = xrBeginSession(session_, &beginInfo);
        if (XR_FAILED(result)) {
            SetFailure(L"xrBeginSession", result);
            return;
        }
        sessionRunning_ = true;
        status_ = L"VR session running. Showing the head-locked FPV test panel.";
    } else if (state == XR_SESSION_STATE_STOPPING && sessionRunning_) {
        const XrResult result = xrEndSession(session_);
        sessionRunning_ = false;
        if (XR_FAILED(result)) SetFailure(L"xrEndSession", result);
        else status_ = L"VR session stopped. Re-enter Quest Link to resume.";
    } else if (state == XR_SESSION_STATE_EXITING ||
               state == XR_SESSION_STATE_LOSS_PENDING) {
        sessionRunning_ = false;
        status_ = L"VR session is closing.";
    }
}

void OpenXrDisplay::RenderFrame() {
    if (!sessionRunning_ || session_ == XR_NULL_HANDLE || swapchain_ == XR_NULL_HANDLE) return;

    XrFrameWaitInfo waitInfo{XR_TYPE_FRAME_WAIT_INFO};
    XrFrameState frameState{XR_TYPE_FRAME_STATE};
    XrResult result = xrWaitFrame(session_, &waitInfo, &frameState);
    if (XR_FAILED(result)) {
        SetFailure(L"xrWaitFrame", result);
        sessionRunning_ = false;
        return;
    }

    XrFrameBeginInfo beginInfo{XR_TYPE_FRAME_BEGIN_INFO};
    result = xrBeginFrame(session_, &beginInfo);
    if (XR_FAILED(result)) {
        SetFailure(L"xrBeginFrame", result);
        sessionRunning_ = false;
        return;
    }

    XrCompositionLayerQuad quad{XR_TYPE_COMPOSITION_LAYER_QUAD};
    XrCompositionLayerBaseHeader* layers[] = {
        reinterpret_cast<XrCompositionLayerBaseHeader*>(&quad)
    };
    std::uint32_t layerCount = 0;

    if (frameState.shouldRender == XR_TRUE) {
        std::uint32_t imageIndex = 0;
        XrSwapchainImageAcquireInfo acquireInfo{XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
        result = xrAcquireSwapchainImage(swapchain_, &acquireInfo, &imageIndex);

        if (XR_SUCCEEDED(result)) {
            XrSwapchainImageWaitInfo waitImage{XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};
            waitImage.timeout = XR_INFINITE_DURATION;
            result = xrWaitSwapchainImage(swapchain_, &waitImage);

            const bool rendered = XR_SUCCEEDED(result) && RenderTestPattern(imageIndex);
            XrSwapchainImageReleaseInfo releaseInfo{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
            const XrResult releaseResult = xrReleaseSwapchainImage(swapchain_, &releaseInfo);

            if (rendered && XR_SUCCEEDED(releaseResult)) {
                quad.space = viewSpace_;
                quad.eyeVisibility = XR_EYE_VISIBILITY_BOTH;
                quad.subImage.swapchain = swapchain_;
                quad.subImage.imageRect.offset = {0, 0};
                quad.subImage.imageRect.extent = {
                    static_cast<std::int32_t>(panelWidth_),
                    static_cast<std::int32_t>(panelHeight_)
                };
                quad.subImage.imageArrayIndex = 0;
                quad.pose.orientation.w = 1.0F;
                quad.pose.position.z = -panelDistanceMeters_;
                quad.size.width = panelWidthMeters_;
                quad.size.height = panelHeight_;
                layerCount = 1;
            } else if (XR_FAILED(result)) {
                SetFailure(L"xrWaitSwapchainImage", result);
            } else if (XR_FAILED(releaseResult)) {
                SetFailure(L"xrReleaseSwapchainImage", releaseResult);
            }
        } else {
            SetFailure(L"xrAcquireSwapchainImage", result);
        }
    }

    XrFrameEndInfo endInfo{XR_TYPE_FRAME_END_INFO};
    endInfo.displayTime = frameState.predictedDisplayTime;
    endInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
    endInfo.layerCount = layerCount;
    endInfo.layers = layerCount > 0 ? layers : nullptr;

    result = xrEndFrame(session_, &endInfo);
    if (XR_FAILED(result)) {
        SetFailure(L"xrEndFrame", result);
        sessionRunning_ = false;
    }
}

void OpenXrDisplay::SetFailure(const wchar_t* operation, XrResult result) {
    status_ = ResultText(operation, result);
}

void OpenXrDisplay::ReleaseResources() noexcept {
    sessionRunning_ = false;
    initialized_ = false;

    renderTargets_.clear();
    swapchainImages_.clear();
    rasterizerState_.Reset();
    vertexBuffer_.Reset();
    inputLayout_.Reset();
    pixelShader_.Reset();
    vertexShader_.Reset();

    if (swapchain_ != XR_NULL_HANDLE) {
        xrDestroySwapchain(swapchain_);
        swapchain_ = XR_NULL_HANDLE;
    }
    if (viewSpace_ != XR_NULL_HANDLE) {
        xrDestroySpace(viewSpace_);
        viewSpace_ = XR_NULL_HANDLE;
    }
    if (session_ != XR_NULL_HANDLE) {
        xrDestroySession(session_);
        session_ = XR_NULL_HANDLE;
    }

    context_.Reset();
    device_.Reset();

    if (instance_ != XR_NULL_HANDLE) {
        xrDestroyInstance(instance_);
        instance_ = XR_NULL_HANDLE;
    }
    systemId_ = XR_NULL_SYSTEM_ID;
}

void OpenXrDisplay::Shutdown() noexcept {
    ReleaseResources();
}

bool OpenXrDisplay::IsReady() const noexcept {
    return initialized_;
}

bool OpenXrDisplay::IsSessionRunning() const noexcept {
    return sessionRunning_;
}

const std::wstring& OpenXrDisplay::Status() const noexcept {
    return status_;
}

} // namespace fpv::xr
