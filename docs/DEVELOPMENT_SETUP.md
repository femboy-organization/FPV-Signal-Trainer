# Настройка среды разработки

## Требования

- Windows 10/11 x64.
- Visual Studio 2022 с workload Desktop development with C++.
- MSVC v143 и Windows SDK.
- Git for Windows.
- Meta Quest Link для аппаратных XR-тестов.

Для CMake и vcpkg отдельная ручная установка не требуется: скрипт сборки находит CMake в PATH/Visual Studio и сам загружает закреплённый vcpkg.

## Одна команда

Из корня репозитория запустите:

    powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1

Скрипт автоматически:
- получает pinned vcpkg из baseline в vcpkg.json;
- запускает bootstrap vcpkg;
- скачивает manifest dependencies;
- использует бинарный кэш библиотек;
- конфигурирует CMake;
- собирает Release;
- запускает unit tests;
- устанавливает EXE в dist.

Для удаления build directory перед сборкой:

    powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1 -Clean

-Clean не удаляет кэш скачанных библиотек.

Если отсутствуют Visual Studio C++ Build Tools или Windows SDK, скрипт завершится с точным сообщением. Эти компиляторы и системные SDK нельзя заменить загрузкой C++ package из vcpkg.

## Зависимости

Список сторонних C++ библиотек находится в vcpkg.json. Сейчас используется OpenXR Loader. Системные D3D11/DXGI/D3DCompiler API берутся из Windows SDK. Meta Quest Link устанавливается отдельно как PCVR runtime.

Подробности: docs/DEPENDENCIES.md.

## Настройка Meta Quest Link / OpenXR runtime

1. Установите и запустите Meta Quest Link.
2. Подключите Quest 3 кабелем USB.
3. В гарнитуре откройте Quest Link и выберите Launch Quest Link.
4. В Meta Quest Link на ПК откройте Settings > General.
5. В разделе OpenXR Runtime нажмите Set Meta Quest Link as active.
6. Закройте FPV Signal Trainer и запустите его заново.
