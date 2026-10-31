#include "xr/openxr_probe.h"

#include <openxr/openxr.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace fpv::xr {
namespace {

constexpr char kD3D11Extension[] = "XR_KHR_D3D11_enable";

bool HasExtension(
    const std::vector<XrExtensionProperties>& properties,
    const char* name) {

    return std::any_of(
        properties.begin(),
        properties.end(),
        [name](const XrExtensionProperties& item) {
            return std::strcmp(item.extensionName, name) == 0;
        });
}

std::wstring WidenAscii(const char* value) {
    std::wstring result;
    if (value == nullptr) {
        return result;
    }

    while (*value != '\0') {
        result.push_back(static_cast<unsigned char>(*value));
        ++value;
    }
    return result;
}

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
    case XR_ERROR_LIMIT_REACHED: return L"XR_ERROR_LIMIT_REACHED";
    case XR_ERROR_OUT_OF_MEMORY: return L"XR_ERROR_OUT_OF_MEMORY";
    case XR_ERROR_NAME_INVALID: return L"XR_ERROR_NAME_INVALID";
    case XR_ERROR_FORM_FACTOR_UNSUPPORTED: return L"XR_ERROR_FORM_FACTOR_UNSUPPORTED";
    case XR_ERROR_FORM_FACTOR_UNAVAILABLE: return L"XR_ERROR_FORM_FACTOR_UNAVAILABLE";
    default: return L"Unknown OpenXR error";
    }
}

std::wstring FormatFailure(const wchar_t* operation, XrResult result) {
    std::wstring message = L"OpenXR: ";
    message += operation;
    message += L" failed: ";
    message += ResultName(result);
    message += L" (code ";
    message += std::to_wstring(static_cast<std::int32_t>(result));
    message += L").";

    if (result == XR_ERROR_RUNTIME_UNAVAILABLE) {
        message += L"\r\nOpen Meta Quest Link > Settings > General > OpenXR Runtime, "
                   L"select 'Set Meta Quest Link as active', then restart this app.";
    } else if (result == XR_ERROR_API_VERSION_UNSUPPORTED) {
        message += L"\r\nThe active runtime rejected OpenXR 1.0.0. Update or reactivate "
                   L"the PCVR runtime in Meta Quest Link.";
    } else if (result == XR_ERROR_EXTENSION_NOT_PRESENT ||
               result == XR_ERROR_EXTENSION_DEPENDENCY_NOT_ENABLED) {
        message += L"\r\nThe active runtime does not provide a required OpenXR extension.";
    } else {
        message += L"\r\nCheck that Meta Quest Link is installed, the Quest is connected "
                   L"over USB, and Meta Quest Link is selected as the active OpenXR runtime.";
    }

    return message;
}

} // namespace

std::wstring QueryRuntimeStatus() {
    std::uint32_t extensionCount = 0;
    XrResult result = xrEnumerateInstanceExtensionProperties(
        nullptr, 0, &extensionCount, nullptr);

    if (XR_FAILED(result)) {
        return FormatFailure(L"xrEnumerateInstanceExtensionProperties", result);
    }

    std::vector<XrExtensionProperties> extensions(extensionCount);
    for (auto& extension : extensions) {
        extension.type = XR_TYPE_EXTENSION_PROPERTIES;
        extension.next = nullptr;
    }

    if (extensionCount > 0) {
        result = xrEnumerateInstanceExtensionProperties(
            nullptr,
            extensionCount,
            &extensionCount,
            extensions.data());

        if (XR_FAILED(result)) {
            return FormatFailure(L"xrEnumerateInstanceExtensionProperties", result);
        }
    }

    const bool hasD3D11 = HasExtension(extensions, kD3D11Extension);

    XrInstanceCreateInfo createInfo{XR_TYPE_INSTANCE_CREATE_INFO};
    createInfo.applicationInfo.apiVersion = XR_MAKE_VERSION(1, 0, 0);
    createInfo.applicationInfo.applicationVersion = 1;
    createInfo.applicationInfo.engineVersion = 1;

    strcpy_s(
        createInfo.applicationInfo.applicationName,
        XR_MAX_APPLICATION_NAME_SIZE,
        "FPV Signal Trainer");
    strcpy_s(
        createInfo.applicationInfo.engineName,
        XR_MAX_ENGINE_NAME_SIZE,
        "Native C++");

    const char* enabledExtensions[] = {kD3D11Extension};
    if (hasD3D11) {
        createInfo.enabledExtensionCount = 1;
        createInfo.enabledExtensionNames = enabledExtensions;
    }

    XrInstance instance = XR_NULL_HANDLE;
    result = xrCreateInstance(&createInfo, &instance);
    if (XR_FAILED(result)) {
        return FormatFailure(L"xrCreateInstance (requested OpenXR 1.0.0)", result);
    }

    XrInstanceProperties instanceProperties{XR_TYPE_INSTANCE_PROPERTIES};
    result = xrGetInstanceProperties(instance, &instanceProperties);

    if (XR_FAILED(result)) {
        xrDestroyInstance(instance);
        return FormatFailure(L"xrGetInstanceProperties", result);
    }

    std::wstring status = L"OpenXR runtime: ";
    status += WidenAscii(instanceProperties.runtimeName);
    status += L" | runtime version ";
    status += std::to_wstring(XR_VERSION_MAJOR(instanceProperties.runtimeVersion));
    status += L".";
    status += std::to_wstring(XR_VERSION_MINOR(instanceProperties.runtimeVersion));
    status += L".";
    status += std::to_wstring(XR_VERSION_PATCH(instanceProperties.runtimeVersion));
    status += hasD3D11
        ? L"\r\nD3D11 graphics extension: available and enabled. Quad layer: OpenXR 1.0 core."
        : L"\r\nD3D11 graphics extension: NOT available from this runtime.";

    xrDestroyInstance(instance);
    return status;
}

} // namespace fpv::xr
