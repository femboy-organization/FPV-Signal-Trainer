# Зависимости и автоматическая сборка

## Автоматически устанавливаемые библиотеки

Все сторонние C++-библиотеки проекта перечисляются в корневом vcpkg.json. Сейчас там закреплён Khronos OpenXR Loader с конкретным baseline. vcpkg manifest mode автоматически разрешает и устанавливает пакеты при сборке.

Для локальной сборки на Windows выполните одну команду из корня репозитория:

    powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1

Скрипт:
1. Читает закреплённый baseline из vcpkg.json.
2. Загружает нужный commit vcpkg в %LOCALAPPDATA%\FPVSignalTrainer\tools.
3. Собирает vcpkg package manager.
4. Включает локальный бинарный кэш пакетов.
5. Выполняет vcpkg install по manifest проекта.
6. Конфигурирует и собирает CMake Release.
7. Запускает тесты и помещает приложение в dist.

Повторные запуски используют уже скачанный код vcpkg и локальный бинарный кэш. Для чистой переконфигурации используйте -Clean; кэш библиотек при этом сохраняется.

## Системные компоненты Windows

Следующие API поставляются Windows SDK и не должны копироваться в папку программы как сторонние DLL:

- Win32;
- Direct3D 11;
- DXGI;
- D3DCompiler;
- Windows Graphics Capture (при реализации capture-модуля);
- C++/WinRT (при использовании WinRT API).

Для сборки нужны Windows 10/11 SDK и MSVC x64 из Visual Studio 2022 с workload Desktop development with C++. Скрипт проверяет наличие инструментов и сообщает, если их нужно установить.

## Runtime Meta Quest Link

Meta Quest Link / Meta PCVR OpenXR runtime не является C++-библиотекой и не распространяется через vcpkg. Его нельзя безопасно встраивать в архив программы: он устанавливается отдельно с официального ПО Meta и регистрируется как активный OpenXR runtime в Windows.

Для запуска:
- установить Meta Quest Link;
- подключить Quest 3 по USB и запустить Quest Link в гарнитуре;
- выбрать Meta Quest Link активным OpenXR runtime.

Проект использует статический triplet x64-windows-static и статический MSVC runtime (/MT), чтобы не требовать отдельных сторонних DLL vcpkg или Visual C++ redistributable для OpenXR Loader. Системные Windows DLL остаются обязательной частью ОС.

## GitHub Actions

CI автоматически:
- загружает pinned vcpkg;
- использует кэш скомпилированных библиотек;
- устанавливает manifest-зависимости;
- собирает приложение MSVC x64;
- запускает CTest;
- формирует ZIP-архив Pre-release.

При обновлении vcpkg.json ключ кэша меняется, а недостающие пакеты загружаются автоматически.

## Добавление новой библиотеки

1. Добавить пакет в vcpkg.json.
2. Запустить scripts/build.ps1.
3. Проверить лицензию и добавить её в THIRD_PARTY_NOTICES.md.
4. Отправить Pull Request. CI выполнит проверку с обновлённым manifest.
