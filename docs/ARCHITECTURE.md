# Архитектура FPV Signal Trainer

**Статус:** Draft, нативная Windows PCVR архитектура.  
**Платформа:** Windows 10/11 x64 + Meta Quest 3.  
**Стек:** C++20, Win32, OpenXR, Direct3D 11, Windows.Graphics.Capture, HLSL, CMake, vcpkg.

## 1. Принцип

Это нативное Windows-приложение, а не игра. Unity и Unreal Engine не используются. Quest 3 работает как PCVR-гарнитура через Meta Quest Link; отдельный APK не требуется для базового PCVR-сценария.

Программа симулирует визуальные эффекты и не воздействует на реальный радиоканал.

## 2. Поток данных

FPV simulator window
-> Windows.Graphics.Capture
-> D3D11 captured texture
-> scaling / aspect correction
-> OSD composition
-> Analog Signal Engine (HLSL)
-> OpenXR swapchain texture
-> OpenXR quad composition layer in VIEW space
-> Meta Quest Link over USB
-> Meta Quest 3

OpenXR возвращает tracking позы головы от runtime Meta Quest Link. Quad layer в VIEW space является целевым способом держать монитор перед взглядом; его необходимо проверить на runtime.

## 3. Модули

### Application Core
Жизненный цикл, конфигурация, логирование, состояния, диагностика и обработка ошибок.

### Win32 UI
Нативная панель управления: статус XR/Link, выбор окна, состояние захвата, настройки монитора, OSD, эффекты и профили. UI не блокирует XR frame loop.

### OpenXR Runtime
Создание instance/system/session, D3D11 graphics binding, swapchain и frame loop. Проверяются расширения XR_KHR_D3D11_enable и XR_KHR_composition_layer_quad. Начальный каркас пока только проверяет доступность runtime и расширений; полноценная session ещё не реализована.

### D3D11 Graphics
GPU-ресурсы и текстуры. По возможности все стадии обработки остаются на GPU; CPU readback избегается.

### Window Capture
Windows.Graphics.Capture: системный picker, GraphicsCaptureItem, Direct3D11CaptureFramePool и D3D11 texture. Нужно корректно обрабатывать COM/WinRT apartment, поток получения кадров, размер frame pool и lifetime текстур. DXGI Desktop Duplication рассматривается как резерв для несовместимых режимов.

### Video Pipeline
Порядок: captured frame -> scaling -> OSD -> analog effects -> XR texture. OSD накладывается до аналоговых помех, чтобы они влияли на всю картинку.

### OSD Editor
Поля: TIMER, RSSI, VOLTAGE, CHANNEL, MODE, LINK QUALITY. Настраиваются включение, положение, размер, цвет, прозрачность и выравнивание. Профили хранятся в версионируемом JSON. Значения без телеметрического источника помечаются как simulated/manual.

### Analog Signal Engine
HLSL эффекты:
- шум и рябь;
- тонкие и сравнительно редкие горизонтальные полосы;
- искажение цветопередачи;
- красно-синее расслоение контуров.

Каждый эффект имеет свой toggle и интенсивность; есть общий master toggle. Не реализуются цифровая пикселизация, искусственные зависания, frame drops и задержка.

### Реальная потеря изображения
NO SIGNAL показывается только при реальном завершении capture source/frame stream. Чёрный кадр сам по себе не является потерей; эффекты не могут инициировать NO SIGNAL.

### Passthrough
Passthrough over Link — отдельная интеграционная возможность. Она должна быть проверена на целевой Quest 3 и версии Link до обещания штатной эксплуатации. Если Passthrough обязателен и не работает приемлемо, архитектуру нужно пересмотреть.

## 4. Структура исходников

src/app — приложение и lifecycle  
src/core — общая логика и логирование  
src/platform/windows — Win32 и Windows APIs  
src/graphics — Direct3D 11  
src/capture — Windows Graphics Capture  
src/xr — OpenXR  
src/video — видеопайплайн  
src/osd — экранная индикация  
src/signal — эффекты и HLSL  
tests — unit и integration tests

## 5. Сборка и зависимости

CMake управляет целями и настройками сборки. vcpkg manifest фиксирует OpenXR loader и baseline. Основной компилятор — MSVC x64. GitHub Actions выполняет configure, build, CTest и packaging. Для CI не требуется XR runtime; тесты с гарнитурой выполняются отдельно.

## 6. Производительность

Измерять частоту захвата, OpenXR frame rate, end-to-end latency, CPU/GPU, время шейдеров, память, температуру и стабильность USB Link. Не заявлять нулевую задержку: её значение определяется измерениями на целевом ноутбуке.

## 7. Ограничения первой версии

Windows PCVR, Quest 3 и проводной Link. Без Wi-Fi, Air Link, HDMI capture cards, отдельного APK, реальных RF-помех, автоматической телеметрии, цифровой пикселизации и искусственных задержек/зависаний.
