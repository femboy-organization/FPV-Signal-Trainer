# Настройка среды разработки

## Требования
- Windows 10/11 x64.
- Visual Studio 2022 с workload Desktop development with C++.
- MSVC v143 и Windows SDK.
- CMake 3.25+.
- Git и vcpkg.
- Для аппаратных XR-тестов: Meta Quest 3 и Meta Quest Link.

## vcpkg
Установите vcpkg в удобный каталог, выполните bootstrap и задайте пользовательскую переменную среды VCPKG_ROOT. Перезапустите терминал после изменения переменной.

## Сборка
Из корня репозитория:
1. cmake --preset windows-msvc
2. cmake --build --preset windows-release --parallel
3. ctest --preset windows-release

CMake preset использует vcpkg manifest mode и статический triplet x64-windows-static. Версии OpenXR loader и baseline фиксируются в vcpkg.json.

## Настройка Meta Quest Link / OpenXR runtime

1. Установите и запустите приложение Meta Quest Link на Windows.
2. Подключите Quest 3 к ноутбуку кабелем USB.
3. В гарнитуре включите Quest Link и запустите Link-соединение.
4. В Meta Quest Link откройте Settings > General.
5. В разделе OpenXR Runtime нажмите Set Meta Quest Link as active.
6. Перезапустите FPV Signal Trainer.

Если активным runtime выбран SteamVR, WMR или другой PCVR runtime, приложение может подключиться к нему вместо Meta Quest Link. Для тестирования Quest должен быть активным runtime.

## Структура исходников
- src/main.cpp — Win32 shell.
- src/xr — OpenXR runtime/session.
- src/graphics — D3D11.
- src/capture — Windows.Graphics.Capture (следующий этап).
- src/video — видеопайплайн.
- src/osd — OSD.
- src/signal — аналоговые эффекты.
- tests — unit tests.

## Важно
Текущий диагностический билд проверяет OpenXR instance и наличие D3D11 extension. Он ещё не создаёт полноценную XR session, swapchain или VR-монитор.

GitHub Actions выполняет сборку и программные тесты, но аппаратные проверки требуют физической гарнитуры.
