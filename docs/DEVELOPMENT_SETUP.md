# Настройка среды разработки

## Требования
- Windows 10/11 x64.
- Visual Studio 2022 с workload Desktop development with C++.
- MSVC v143 и Windows SDK.
- CMake 3.25+.
- Git и vcpkg.

## vcpkg
Установите vcpkg в удобный каталог, выполните bootstrap и задайте пользовательскую переменную среды VCPKG_ROOT. Перезапустите терминал после изменения переменной.

## Сборка
Из корня репозитория:
1. cmake --preset windows-msvc
2. cmake --build --preset windows-release --parallel
3. ctest --preset windows-release

CMake preset использует vcpkg manifest mode и статический triplet x64-windows-static. Версии OpenXR loader и baseline фиксируются в vcpkg.json.

## Структура исходников
- src/main.cpp — старт приложения и Win32 shell.
- src/xr — OpenXR runtime/session.
- src/graphics — D3D11.
- src/capture — Windows.Graphics.Capture (следующий этап).
- src/video — видеопайплайн.
- src/osd — OSD.
- src/signal — аналоговые эффекты.
- tests — unit tests.

## Quest Link
Для аппаратных тестов установите Meta Quest Link, подключите Quest 3 по USB и проверьте результат USB Test. Wi-Fi/Air Link не используются.

GitHub Actions выполняет сборку и программные тесты, но не имеет физической гарнитуры.
