# FPV Signal Trainer

Нативное Windows PCVR-приложение для студентов направления беспилотных летательных систем. Цель — вывести окно FPV-симулятора на виртуальный монитор в Meta Quest 3 и моделировать визуальные эффекты аналогового видеоканала.

**Текущий этап:** OpenXR session, D3D11 swapchain и head-locked тестовая панель реализованы в коде. Требуется аппаратная проверка на Quest 3.

## Технологии

- C++20 / Win32 API;
- OpenXR;
- Direct3D 11 / DXGI / HLSL;
- Windows Graphics Capture (следующий этап);
- CMake + vcpkg + MSVC;
- GitHub Actions.

Unity и Unreal Engine не используются. Рабочий сценарий — проводной Meta Quest Link по USB. Wi-Fi/Air Link и HDMI-карты захвата не входят в проект.

## Автоматическая загрузка библиотек и сборка

Для локальной сборки на Windows установите Visual Studio 2022 с workload Desktop development with C++ и Windows SDK. Затем из корня репозитория выполните:

    powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1

Скрипт сам скачает закреплённую версию vcpkg, установит библиотеки из vcpkg.json, настроит CMake, соберёт программу, запустит тесты и положит EXE в dist.

Все внешние C++-зависимости перечисляются в vcpkg.json. При добавлении новой библиотеки vcpkg автоматически загрузит её на следующей сборке. Повторные сборки используют локальный кэш.

Подробности: docs/DEPENDENCIES.md.

## Meta Quest Link

Meta Quest Link — отдельный runtime, не C++ package. Установите официальное приложение Meta Quest Link, подключите Quest 3 по USB, запустите Quest Link в гарнитуре и назначьте Meta Quest Link активным OpenXR runtime.

## Текущий статус

В коде реализованы OpenXR instance/system/session, D3D11 graphics binding, swapchain и тестовая цветовая панель через quad composition layer в VIEW space. Передача окна симулятора, OSD и аналоговые эффекты — следующие этапы. Успешная компиляция в CI не заменяет проверку на физической гарнитуре.
