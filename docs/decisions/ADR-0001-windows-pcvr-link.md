# ADR-0001: Native Windows PCVR через Meta Quest Link

- Статус: Proposed — проверяется прототипом.
- Дата: 2026-10-31.

## Контекст
Нужно выводить окно FPV-симулятора с Windows-ноутбука на Meta Quest 3. Есть USB-A–USB-C и USB-C–USB-C, HDMI capture cards отсутствуют, Wi-Fi не нужен. Для виртуального монитора игровой движок избыточен.

## Решение-кандидат
Нативное Windows x64 приложение на C++20:
- Win32 API для desktop UI;
- OpenXR для VR runtime;
- Direct3D 11 для графики и swapchain;
- Windows.Graphics.Capture для захвата окна;
- HLSL для аналоговых эффектов;
- CMake + vcpkg для сборки/зависимостей;
- MSVC x64 и GitHub Actions для CI.

Meta Quest Link по USB отвечает за PCVR video transport и tracking. Базовая схема не требует отдельного Android APK.

Целевой вывод — OpenXR quad composition layer в VIEW space. На первом этапе нужно проверить runtime, затем реализовать session, D3D11 binding и swapchain.

## Плюсы
Нативное приложение без Unity/Unreal, прямой доступ к Windows capture и D3D11 textures, полный контроль над графическим pipeline, единая Windows-сборка и автоматическая CI.

## Риски
Низкоуровневая сложность C++, OpenXR lifecycle и D3D11 synchronization; конкуренция за GPU с симулятором; зависимость от USB Link; Passthrough over Link требует проверки.

## Условия принятия
Проверить кабели, native OpenXR instance/session, D3D11 graphics binding, quad layer, head-locked поведение, захват основного симулятора, Passthrough (если обязателен), задержку и температуру. ADR получает Accepted только после успешного теста на физическом Quest 3.
