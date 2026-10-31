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

1. Установите и запустите приложение Meta Quest Link / Meta Horizon Link на Windows.
2. Подключите Quest 3 кабелем USB.
3. В гарнитуре откройте Quest Link и выберите Launch Quest Link, чтобы PCVR-соединение действительно запустилось.
4. В Meta Quest Link на ПК откройте Settings > General.
5. В разделе OpenXR Runtime нажмите Set Meta Quest Link as active.
6. Закройте FPV Signal Trainer и запустите его заново.

### Если приложение сообщает XR_ERROR_RUNTIME_FAILURE

Следующая диагностическая версия показывает:
- какой OpenXR manifest выбран;
- выбран ли он переменной XR_RUNTIME_JSON или через реестр Windows;
- существует ли JSON-файл;
- создаётся ли базовый OpenXR instance без графических расширений;
- создаётся ли отдельный instance с XR_KHR_D3D11_enable.

OpenXR Loader использует XR_RUNTIME_JSON как override; если переменная задана, она имеет приоритет над стандартным ActiveRuntime в реестре.

Путь ActiveRuntime для 64-битного приложения:
HKEY_LOCAL_MACHINE\SOFTWARE\Khronos\OpenXR\1\ActiveRuntime

Не редактируйте реестр вслепую. Если manifest отсутствует или указывает на старую установку, сначала запустите Meta Quest Link и используйте его настройку Set Meta Quest Link as active. Если это не помогает, сохраните диагностический текст приложения и проверьте/восстановите установку Meta Quest Link.

Официальная документация:
- https://developers.meta.com/vr/documentation/unreal/unreal-link/
- https://registry.khronos.org/OpenXR/specs/1.1/loader.html

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
Текущая диагностическая сборка проверяет создание OpenXR instance и D3D11 extension. Она ещё не создаёт полноценную XR session, swapchain или VR-монитор.

GitHub Actions выполняет сборку и программные тесты, но аппаратные проверки требуют физической гарнитуры.
