# Сторонние компоненты и материалы

MIT в корне репозитория распространяется на оригинальный код проекта. Она не меняет лицензии внешних SDK и библиотек.

| Компонент | Назначение | Условия |
|---|---|---|
| Khronos OpenXR SDK / Loader | XR headers and loader | Проверять LICENSE и notices конкретной версии; upstream OpenXR SDK распространяется под Apache-2.0 |
| Windows SDK | Win32, D3D11, Windows Graphics Capture | Microsoft SDK/platform terms |
| vcpkg | Управление зависимостями | MIT |
| CMake | Build system | BSD-3-Clause |
| MSVC | Компилятор | Microsoft Visual Studio terms |
| Meta Quest Link | PCVR runtime | Условия Meta |

При добавлении зависимостей и ресурсов фиксировать точную версию, источник, лицензию и атрибуцию. Не коммитить закрытые SDK или runtime DLL без разрешения на распространение.
