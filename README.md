# FPV Signal Trainer

Учебное нативное Windows-приложение для студентов направления беспилотных летательных систем. Программа предназначена для вывода изображения FPV-симулятора на виртуальный монитор в Meta Quest 3 и программной имитации помех аналогового видеоканала.

**Текущий статус:** создан начальный C++/CMake каркас и CI. Полноценная VR-панель и захват окна ещё не реализованы.

## Концепция

Студент управляет симулятором на ноутбуке и видит его изображение на виртуальном мониторе перед глазами в Meta Quest 3.

- Windows 10/11 x64.
- Только проводной Meta Quest Link по USB.
- Нативное приложение C++20.
- OpenXR для VR.
- Direct3D 11 для графики.
- Windows Graphics Capture для будущего захвата окна.
- HLSL для будущих визуальных эффектов.
- CMake + vcpkg + MSVC.
- GitHub Actions для сборки и тестов.
- Unity и Unreal Engine не используются.
- Wi-Fi, Air Link и HDMI-карты захвата не входят в проект.
- Отдельный APK для Quest не входит в базовый PCVR-вариант.

Конкретные USB-кабели и порты требуют проверки. Passthrough over Link — отдельный технический риск и должен быть проверен на реальном Quest 3.

## Планируемые возможности

- Head-locked виртуальный монитор с настройкой размера и положения.
- Захват выбранного окна Windows.
- Аналоговые эффекты: шум/рябь, тонкие редкие горизонтальные полосы, искажение цветов, красно-синее цветовое расслоение.
- Индивидуальные и общий переключатели эффектов.
- Редактор OSD с пользовательскими профилями.
- NO SIGNAL только при реальной потере источника.

Искусственные фризы, потеря кадров, задержка и цифровая блочная пикселизация исключены.

## Сборка на Windows

Требуется Visual Studio 2022 с компонентом Desktop development with C++, CMake 3.25+ и vcpkg. Установите vcpkg и задайте переменную среды VCPKG_ROOT.

Из корня репозитория выполните:
- cmake --preset windows-msvc
- cmake --build --preset windows-release --parallel
- ctest --preset windows-release

Пресет использует MSVC x64 и vcpkg manifest mode. Запуск VR требует установленного Meta Quest Link и подключённой гарнитуры.

## GitHub Actions

Workflow windows-ci выполняет Windows x64 configure, build, unit tests и упаковку ZIP. Успешная сборка доступна как Actions artifact; push тега вида v* создаёт GitHub Release.

CI runner не имеет подключённого Quest 3, поэтому hardware tests Link, tracking, quad layer и Passthrough выполняются отдельно.

## Статус первого каркаса

Уже есть:
- CMake проект;
- OpenXR runtime availability probe;
- Direct3D 11 device probe;
- Win32 диагностическое окно;
- базовые unit tests для настроек эффектов;
- GitHub Actions workflow.

Ещё нет:
- полноценной OpenXR session и swapchain;
- quad composition layer;
- Windows Graphics Capture;
- передачи видеокадров на монитор;
- OSD editor;
- HLSL эффектов;
- Scenario Manager.

## Документация

- docs/ARCHITECTURE.md
- docs/TECHNICAL_SPECIFICATION.md
- docs/ROADMAP.md
- docs/DEVELOPMENT_SETUP.md
- docs/TEST_PLAN.md
- docs/RELEASE_PROCESS.md
- docs/decisions/ADR-0001-windows-pcvr-link.md

## Лицензия

Оригинальный код проекта распространяется по MIT License. У сторонних SDK и библиотек собственные условия; см. THIRD_PARTY_NOTICES.md.
