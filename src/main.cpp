#include "graphics/d3d11_probe.h"
#include "xr/openxr_probe.h"

#include <windows.h>

namespace {

constexpr wchar_t kWindowClass[] = L"FPVSignalTrainerWindow";
constexpr wchar_t kWindowTitle[] = L"FPV Signal Trainer - Native development build";

LRESULT CALLBACK WindowProcedure(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam) {

    switch (message) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(window, message, wParam, lParam);
    }
}

void AddStatusLabel(HWND parent, int y, int height, const wchar_t* text) {
    const HWND control = CreateWindowExW(
        0,
        L"STATIC",
        text,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        28,
        y,
        900,
        height,
        parent,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr);

    if (control != nullptr) {
        SendMessageW(
            control,
            WM_SETFONT,
            reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)),
            TRUE);
    }
}

} // namespace

int WINAPI wWinMain(
    HINSTANCE instance,
    HINSTANCE,
    PWSTR,
    int showCommand) {

    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = WindowProcedure;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    windowClass.lpszClassName = kWindowClass;

    if (RegisterClassExW(&windowClass) == 0 &&
        GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return 1;
    }

    const HWND window = CreateWindowExW(
        0,
        kWindowClass,
        kWindowTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        980,
        500,
        nullptr,
        nullptr,
        instance,
        nullptr);

    if (window == nullptr) {
        return 1;
    }

    const std::wstring graphicsStatus = fpv::graphics::QueryD3D11Status();
    const std::wstring xrStatus = fpv::xr::QueryRuntimeStatus();

    AddStatusLabel(window, 28, 34, L"FPV Signal Trainer - native C++ foundation");
    AddStatusLabel(window, 82, 34, graphicsStatus.c_str());
    AddStatusLabel(window, 128, 112, xrStatus.c_str());
    AddStatusLabel(
        window,
        252,
        48,
        L"Window capture: planned next milestone (Windows.Graphics.Capture).");
    AddStatusLabel(
        window,
        326,
        44,
        L"This build checks the local graphics device and OpenXR runtime.");
    AddStatusLabel(
        window,
        370,
        44,
        L"It does not yet display the simulator in the headset.");

    ShowWindow(window, showCommand);
    UpdateWindow(window);

    MSG message{};
    while (true) {
        const BOOL result = GetMessageW(&message, nullptr, 0, 0);
        if (result == 0) {
            return static_cast<int>(message.wParam);
        }
        if (result == -1) {
            return 1;
        }

        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
}
