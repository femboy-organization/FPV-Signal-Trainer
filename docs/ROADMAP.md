# Дорожная карта

Сроки определяются после проверки оборудования.

## Phase 0 — Аппаратная проверка
- Проверить Meta Quest Link на доступных USB-A–USB-C и USB-C–USB-C кабелях.
- Зафиксировать результаты USB Test и стабильность.
- Проверить native OpenXR PCVR sample.
- Проверить Passthrough over Link.
- Оценить совместную нагрузку симулятора и VR-приложения.

## Phase 1 — Native Windows foundation
- C++20 / CMake / vcpkg.
- Win32 diagnostics window.
- D3D11 device initialization.
- OpenXR runtime and extension probe.
- Unit tests.
- GitHub Actions Windows build/test/package.

**Текущий этап:** начальная основа создана. Это пока не VR-монитор.

## Phase 2 — OpenXR monitor
- D3D11 graphics binding.
- OpenXR session и event loop.
- Swapchain.
- Quad composition layer в VIEW space.
- Head-locked панель и настройки размера/положения.

## Phase 3 — Window Capture
- Windows.Graphics.Capture picker.
- D3D11 frame textures.
- Передача кадров в XR pipeline.
- Обработка закрытия окна, изменения размера и восстановления потока.

## Phase 4 — OSD Editor
- Элементы OSD и их параметры.
- Позиционирование и визуальные настройки.
- JSON profiles и маркировка демонстрационных значений.

## Phase 5 — Analog Signal Engine
- Шум и рябь.
- Редкие тонкие горизонтальные полосы.
- Искажение цветов.
- Красно-синее расслоение.
- Независимые/общий переключатели.
- HLSL profiling.

## Phase 6 — Scenario Manager
Профили OSD и эффектов, сохранение/загрузка, импорт/экспорт, версия схемы.

## Phase 7 — Passthrough и эргономика
Проверить видимость клавиатуры и органов управления, комфорт длительной работы. Если Passthrough обязателен, но не работает приемлемо через Link, пересмотреть архитектуру.

## Phase 8 — Стабилизация и 1.0
Длительные тесты, диагностика, упаковка, инструкции, проверка сторонних лицензий, Release Candidate и стабильный релиз.
