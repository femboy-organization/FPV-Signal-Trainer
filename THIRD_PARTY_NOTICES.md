# Сторонние компоненты и материалы

MIT в корне репозитория распространяется на оригинальный код проекта. Она не меняет лицензии внешних SDK и библиотек.

| Компонент | Использование | Условия |
|---|---|---|
| Khronos OpenXR SDK / Loader | XR headers and loader, устанавливается через vcpkg manifest | Apache-2.0; см. license/notices конкретной версии пакета |
| Windows SDK | Win32, D3D11, DXGI, D3DCompiler, Windows Graphics Capture | Microsoft SDK/platform terms |
| vcpkg | Автоматическое управление и установка зависимостей | MIT |
| CMake | Build system | BSD-3-Clause |
| MSVC | Компилятор и C++ runtime toolchain | Microsoft Visual Studio terms |
| Meta Quest Link | Отдельный PCVR OpenXR runtime | Условия Meta; не распространяется этим проектом |

Проект использует triplet x64-windows-static и статический MSVC runtime (/MT), чтобы сторонние C++ зависимости не требовали набора DLL рядом с приложением.

Системные Windows DLL и Meta Quest Link runtime не входят в ZIP. Meta Quest Link требуется установить отдельно с официального источника Meta и выбрать активным OpenXR runtime.

При добавлении зависимостей фиксируйте точную версию, источник, лицензию и атрибуцию. Не коммитьте закрытые SDK или runtime DLL без разрешения на распространение.
