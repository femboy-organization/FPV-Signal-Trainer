# FPV Signal Trainer

Нативное Windows PCVR-приложение для студентов направления беспилотных летательных систем. Цель — вывести окно FPV-симулятора на виртуальный монитор в Meta Quest 3 и моделировать визуальные эффекты аналогового видеоканала.

**Текущий этап:** реализуется OpenXR-сессия, D3D11 swapchain и head-locked тестовая панель.

## Технологии
- C++20 / Win32 API;
- OpenXR;
- Direct3D 11 / DXGI;
- HLSL;
- Windows Graphics Capture (следующий этап);
- CMake + vcpkg + MSVC;
- GitHub Actions.

Unity и Unreal Engine не используются. Рабочий сценарий — проводной Meta Quest Link по USB. Wi-Fi/Air Link и HDMI-карты захвата не входят в проект.

## Функции
1. Виртуальная панель в пространстве VIEW, закреплённая перед взглядом.
2. Захват выбранного окна Windows.
3. OSD Editor и сохранение профилей.
4. Аналоговые эффекты: шум/рябь, тонкие горизонтальные полосы, цветовые искажения, красно-синее расслоение.
5. NO SIGNAL только при реальной потере источника.

## Сборка Windows
Требуется Visual Studio 2022 (Desktop development with C++), CMake 3.25+, Git и vcpkg. Установите vcpkg, задайте VCPKG_ROOT, затем из корня репозитория:

    cmake --preset windows-msvc
    cmake --build --preset windows-release --parallel
    ctest --preset windows-release

Запуск XR требует Meta Quest Link, активного OpenXR runtime и подключённой Quest 3.

## Статус
В текущем этапе создаётся OpenXR instance/session, D3D11 swapchain и тестовая цветовая панель через quad composition layer. Захват окна, OSD и эффекты ещё не реализованы. Требуется проверка на физической Quest 3.
