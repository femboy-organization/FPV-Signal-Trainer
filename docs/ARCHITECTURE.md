# Архитектура FPV Signal Trainer

Нативное Windows PCVR-приложение: C++20, Win32, OpenXR, Direct3D 11, HLSL и CMake.

## Текущий вертикальный срез
1. OpenXR instance с XR_KHR_D3D11_enable.
2. Получение HMD system.
3. Получение D3D11 требований runtime и выбор DXGI adapter по LUID.
4. Создание ID3D11Device на требуемом adapter.
5. Создание OpenXR session с XrGraphicsBindingD3D11KHR.
6. Создание reference space VIEW.
7. Создание D3D11 swapchain images и render target views.
8. Рендер тестовой цветовой/прицельной панели через HLSL.
9. Отправка XrCompositionLayerQuad в VIEW space.

Панель следует за направлением взгляда. На этой стадии отображается тестовая панель, не изображение симулятора.

## Следующие модули
### Window Capture
Windows.Graphics.Capture: выбор окна, кадры через Direct3D11CaptureFramePool. Поверхность захвата должна оставаться в D3D11/GPU pipeline без CPU readback. Не выполнять тяжёлую обработку в FrameArrived; кадры нужно забирать и быстро освобождать.

### Video Pipeline
Captured texture -> aspect/scale -> OSD composition -> analog effects -> XR swapchain image.

### OSD
Редактируемые TIMER, RSSI, VOLTAGE, CHANNEL, MODE, LINK QUALITY. Значения без настоящего телеметрического источника маркируются simulated/manual. OSD компонуется до помех.

### Analog Signal Engine
HLSL effects: noise/static, редкие тонкие horizontal bands, color distortion, red/blue chromatic separation. Независимые и общий выключатели. Без искусственных freeze, frame drops и задержки.

### Passthrough
Passthrough over Link остаётся аппаратным риском и требует проверки на конкретной версии Meta Quest Link.

## Правила OpenXR/D3D11
- Перед xrCreateSession вызвать xrGetD3D11GraphicsRequirementsKHR.
- Использовать adapter LUID, предоставленный runtime.
- Создавать D3D11 graphics binding с корректным device.
- OpenXR swapchain texture использовать после acquire/wait и до release.
- Для каждого xrWaitFrame вызывать xrBeginFrame и xrEndFrame.
- Не выполнять тяжёлую CPU-обработку внутри XR frame loop.

## Источники
- Khronos OpenXR specification.
- xrGetD3D11GraphicsRequirementsKHR reference.
- Microsoft Windows.Graphics.Capture screen capture documentation.
