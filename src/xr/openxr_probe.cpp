#include "xr/openxr_probe.h"

#include <openxr/openxr.h>
#include <windows.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace fpv::xr {
namespace {

constexpr char kD3D11Extension[] = "XR_KHR_D3D11_enable";
constexpr wchar_t kOpenXrRegistryKey[] = L"SOFTWARE\\Khronos\\OpenXR\\1";

struct RuntimeManifestInfo final {
    std::wstring path;
    std::wstring source;
    bool pathConfigured{false};
    bool fileExists{false};
};

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

std::wstring ReadEnvironmentVariable(const wchar_t* name) {
    const DWORD required = GetEnvironmentVariableW(name, nullptr, 0);
    if (required == 0) {
        return {};
    }

    std::vector<wchar_t> buffer(required, L'\0');
    const DWORD written = GetEnvironmentVariableW(
        name,
        buffer.data(),
        static_cast<DWORD>(buffer.size()));

    if (written == 0 || written >= buffer.size()) {
        return {};
    }

    return std::wstring(buffer.data(), written);
}

std::wstring ReadActiveRuntimeFromRegistry() {
    HKEY key = nullptr;
    const LONG openResult = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        kOpenXrRegistryKey,
        0,
        KEY_QUERY_VALUE | KEY_WOW64_64KEY,
        &key);

    if (openResult != ERROR_SUCCESS) {
        return {};
    }

    DWORD type = 0;
    DWORD byteCount = 0;
    const LONG sizeResult = RegQueryValueExW(
        key,
        L"ActiveRuntime",
        nullptr,
        &type,
        nullptr,
        &byteCount);

    if (sizeResult != ERROR_SUCCESS ||
        (type != REG_SZ && type != REG_EXPAND_SZ) ||
        byteCount < sizeof(wchar_t)) {
        RegCloseKey(key);
        return {};
    }

    std::vector<wchar_t> buffer(
        byteCount / sizeof(wchar_t) + 1,
        L'\0');

    const LONG readResult = RegQueryValueExW(
        key,
        L"ActiveRuntime",
        nullptr,
        &type,
        reinterpret_cast<LPBYTE>(buffer.data()),
        &byteCount);

    RegCloseKey(key);

    if (readResult != ERROR_SUCCESS) {
        return {};
    }

    std::wstring value(buffer.data());

    if (type == REG_EXPAND_SZ && !value.empty()) {
        const DWORD expandedLength = ExpandEnvironmentStringsW(
            value.c_str(), nullptr, 0);

        if (expandedLength > 0) {
            std::vector<wchar_t> expanded(expandedLength, L'\0');
            const DWORD expandedWritten = ExpandEnvironmentStringsW(
                value.c_str(),
                expanded.data(),
                expandedLength);

            if (expandedWritten > 0 && expandedWritten <= expanded.size()) {
                value.assign(expanded.data());
            }
        }
    }

    return value;
}

RuntimeManifestInfo GetRuntimeManifestInfo() {
    RuntimeManifestInfo info;

    // XR_RUNTIME_JSON takes precedence over the Windows ActiveRuntime registry value.
    info.path = ReadEnvironmentVariable(L"XR_RUNTIME_JSON");
    if (!info.path.empty()) {
        info.source = L"XR_RUNTIME_JSON environment override";
    } else {
        info.path = ReadActiveRuntimeFromRegistry();
        if (!info.path.empty()) {
            info.source = L"HKLM 64-bit OpenXR ActiveRuntime registry value";
        }
    }

    info.pathConfigured = !info.path.empty();
    if (info.pathConfigured) {
        const DWORD attributes = GetFileAttributesW(info.path.c_str());
        info.fileExists =
            attributes != INVALID_FILE_ATTRIBUTES &&
            (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }

    return info;
}

std::wstring DescribeRuntimeManifest(const RuntimeManifestInfo& info) {
    std::wstring text = L"\r\nRuntime manifest: ";

    if (!info.pathConfigured) {
        text += L"not configured.";
        text += L"\r\nExpected registry value: HKLM\\SOFTWARE\\Khronos\\OpenXR\\1\\ActiveRuntime.";
        return text;
    }

    text += info.path;
    text += L"\r\nManifest source: ";
    text += info.source;
    text += info.fileExists
        ? L"\r\nManifest file: FOUND."
        : L"\r\nManifest file: NOT FOUND. Repair/reinstall the selected OpenXR runtime.";

    return text;
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
        message += L"\r\nThe loader could not start the selected runtime. Start Meta Quest Link and select it as active.";
    } else if (result == XR_ERROR_API_VERSION_UNSUPPORTED) {
        message += L"\r\nThe selected runtime rejected OpenXR 1.0.0. Update or repair Meta Quest Link.";
    } else if (result == XR_ERROR_RUNTIME_FAILURE) {
        message += L"\r\nThe runtime was discovered but returned an internal failure. Check its manifest path below, restart Meta Quest Link, and reconnect Quest Link in the headset.";
    } else if (result == XR_ERROR_EXTENSION_NOT_PRESENT ||
               result == XR_ERROR_EXTENSION_DEPENDENCY_NOT_ENABLED) {
        message += L"\r\nThe selected runtime does not provide a required extension.";
    } else {
        message += L"\r\nCheck the active OpenXR runtime and the Meta Quest Link installation.";
    }

    return message;
}

bool CreateInstance(
    bool enableD3D11,
    XrInstance* instance,
    XrResult* result) {

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
    if (enableD3D11) {
        createInfo.enabledExtensionCount = 1;
        createInfo.enabledExtensionNames = enabledExtensions;
    }

    *instance = XR_NULL_HANDLE;
    *result = xrCreateInstance(&createInfo, instance);
    return XR_SUCCEEDED(*result);
}

} // namespace

std::wstring QueryRuntimeStatus() {
    const RuntimeManifestInfo manifest = GetRuntimeManifestInfo();

    // First create a minimal instance without graphics extensions.
    XrInstance baseInstance = XR_NULL_HANDLE;
    XrResult result = XR_SUCCESS;

    if (!CreateInstance(false, &baseInstance, &result)) {
        std::wstring message = FormatFailure(
            L"xrCreateInstance without graphics extensions (OpenXR 1.0.0)",
            result);
        message += DescribeRuntimeManifest(manifest);
        return message;
    }

    XrInstanceProperties instanceProperties{XR_TYPE_INSTANCE_PROPERTIES};
    result = xrGetInstanceProperties(baseInstance, &instanceProperties);

    if (XR_FAILED(result)) {
        xrDestroyInstance(baseInstance);
        std::wstring message = FormatFailure(L"xrGetInstanceProperties", result);
        message += DescribeRuntimeManifest(manifest);
        return message;
    }

    const std::wstring runtimeName = WidenAscii(instanceProperties.runtimeName);
    const std::wstring runtimeVersion =
        std::to_wstring(XR_VERSION_MAJOR(instanceProperties.runtimeVersion)) + L"." +
        std::to_wstring(XR_VERSION_MINOR(instanceProperties.runtimeVersion)) + L"." +
        std::to_wstring(XR_VERSION_PATCH(instanceProperties.runtimeVersion));

    xrDestroyInstance(baseInstance);

    std::uint32_t extensionCount = 0;
    result = xrEnumerateInstanceExtensionProperties(
        nullptr, 0, &extensionCount, nullptr);

    if (XR_FAILED(result)) {
        std::wstring message = FormatFailure(
            L"xrEnumerateInstanceExtensionProperties",
            result);
        message += DescribeRuntimeManifest(manifest);
        return message;
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
            std::wstring message = FormatFailure(
                L"xrEnumerateInstanceExtensionProperties",
                result);
            message += DescribeRuntimeManifest(manifest);
            return message;
        }
    }

    const bool hasD3D11 = HasExtension(extensions, kD3D11Extension);

    std::wstring status =
        L"OpenXR base instance: OK\r\nRuntime: " + runtimeName +
        L" | version " + runtimeVersion;

    if (!hasD3D11) {
        status += L"\r\nD3D11 graphics extension: NOT AVAILABLE. This runtime cannot be used by the planned D3D11 renderer.";
        status += DescribeRuntimeManifest(manifest);
        return status;
    }

    // Test the graphics extension separately from runtime discovery.
    XrInstance d3d11Instance = XR_NULL_HANDLE;
    if (!CreateInstance(true, &d3d11Instance, &result)) {
        status += L"\r\nD3D11 extension: advertised by runtime, but instance creation with it FAILED.";
        status += L"\r\n";
        status += ResultName(result);
        status += L" (code ";
        status += std::to_wstring(static_cast<std::int32_t>(result));
        status += L").";
        status += DescribeRuntimeManifest(manifest);
        return status;
    }

    xrDestroyInstance(d3d11Instance);

    status += L"\r\nD3D11 graphics extension: available and instance creation succeeded.";
    status += L"\r\nQuad composition layer is part of OpenXR 1.0 core.";
    status += DescribeRuntimeManifest(manifest);
    return status;
}

} // namespace fpv::xr
