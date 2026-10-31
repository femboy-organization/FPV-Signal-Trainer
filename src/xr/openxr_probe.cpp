#include "xr/openxr_probe.h"

#include <openxr/openxr.h>

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

namespace fpv::xr {
namespace {

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

} // namespace

std::wstring QueryRuntimeStatus() {
    std::uint32_t count = 0;
    XrResult result = xrEnumerateInstanceExtensionProperties(
        nullptr, 0, &count, nullptr);

    if (XR_FAILED(result)) {
        return L"OpenXR: runtime unavailable. Start Meta Quest Link and connect Quest 3.";
    }

    std::vector<XrExtensionProperties> extensions(count);
    for (auto& extension : extensions) {
        extension.type = XR_TYPE_EXTENSION_PROPERTIES;
        extension.next = nullptr;
    }

    if (count > 0) {
        result = xrEnumerateInstanceExtensionProperties(
            nullptr, count, &count, extensions.data());

        if (XR_FAILED(result)) {
            return L"OpenXR: could not enumerate runtime extensions.";
        }
    }

    XrInstanceCreateInfo createInfo{XR_TYPE_INSTANCE_CREATE_INFO};
    createInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
    strcpy_s(
        createInfo.applicationInfo.applicationName,
        XR_MAX_APPLICATION_NAME_SIZE,
        "FPV Signal Trainer");
    strcpy_s(
        createInfo.applicationInfo.engineName,
        XR_MAX_ENGINE_NAME_SIZE,
        "Native C++");

    XrInstance instance = XR_NULL_HANDLE;
    result = xrCreateInstance(&createInfo, &instance);
    if (XR_FAILED(result)) {
        return L"OpenXR: extensions found, but instance creation failed.";
    }

    XrInstanceProperties instanceProperties{XR_TYPE_INSTANCE_PROPERTIES};
    result = xrGetInstanceProperties(instance, &instanceProperties);
    xrDestroyInstance(instance);

    if (XR_FAILED(result)) {
        return L"OpenXR: runtime found; runtime properties unavailable.";
    }

    const bool hasD3D11 = HasExtension(extensions, "XR_KHR_D3D11_enable");
    const bool hasQuad = HasExtension(extensions, "XR_KHR_composition_layer_quad");

    std::wstring status = L"OpenXR runtime: ";
    for (const char* p = instanceProperties.runtimeName; *p != '\0'; ++p) {
        status.push_back(static_cast<unsigned char>(*p));
    }
    status += hasD3D11 ? L" | D3D11: yes" : L" | D3D11: no";
    status += hasQuad ? L" | Quad layer: yes" : L" | Quad layer: no";
    return status;
}

} // namespace fpv::xr
