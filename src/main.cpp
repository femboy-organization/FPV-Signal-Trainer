#include "xr/openxr_display.h"

#include <windows.h>

#include <string>

namespace {

constexpr wchar_t kWindowClass[] = L"FPVSignalTrainerWindow";
constexpr wchar_t kWindowTitle[] = L"FPV Signal Trainer";
HWND gStatusControl = nullptr;

LRESULT CALLBACK WindowProcedure(
    HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

void UpdateStatus(const std::wstring& status) {
    if (gStatusControl != nullptr) {
        SetWindowTextW(gStatusControl, status.c_str());
    }
}

HWND CreateMainWindow(HINSTANCE instance, int showCommand) {
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = WindowProcedure;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    windowClass.lpszClassName = kWindowClass;

    if (RegisterClassExW(&windowClass) == 0 &&
        GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return nullptr;
    }

    const HWND window = CreateWindowExW(
        0, kWindowClass, kWindowTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 780, 420,
        nullptr, nullptr, instance, nullptr);
    if (window == nullptr) return nullptr;

    CreateWindowExW(
        0, L"STATIC",
        L"FPV Signal Trainer - native OpenXR application",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        24, 24, 700, 30, window, nullptr, instance, nullptr);

    gStatusControl = CreateWindowExW(
        0, L"STATIC", L"Initializing OpenXR...",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        24, 72, 710, 280, window, nullptr, instance, nullptr);

    if (gStatusControl != nullptr) {
        SendMessageW(gStatusControl, WM_SETFONT,
            reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
    }

    ShowWindow(window, showCommand);
    UpdateWindow(window);
    return window;
}

} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    const HWND window = CreateMainWindow(instance, showCommand);
    if (window == nullptr) return 1;

    fpv::xr::OpenXrDisplay display;
    const bool initialized = display.Initialize();
    UpdateStatus(display.Status());

    if (!initialized) {
        MSG message{};
        while (true) {
            const BOOL result = GetMessageW(&message, nullptr, 0, 0);
            if (result == 0) return static_cast<int>(message.wParam);
            if (result == -1) return 1;
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }

    MSG message{};
    bool running = true;
    while (running) {
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                running = false;
                break;
            }
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        if (!running) break;

        display.PollEvents();
        display.RenderFrame();

        if (!display.IsSessionRunning()) {
            UpdateStatus(display.Status());
            Sleep(10);
        }
    }

    display.Shutdown();
    return static_cast<int>(message.wParam);
}
