# План тестирования

## CI
- CMake configure на Windows x64.
- MSVC Release build.
- CTest.
- ZIP artifact packaging.

## Unit tests
- Эффекты выключены по умолчанию.
- Интенсивность ограничена диапазоном 0..1.
- NaN нормализуется в безопасное значение.
- По мере реализации добавить тесты JSON profiles и capture state machine.

## OpenXR hardware tests
На физическом Quest 3 с Meta Quest Link:
- runtime detection;
- OpenXR instance/system;
- D3D11 extension;
- quad layer extension;
- D3D11 graphics binding и session;
- swapchain;
- head-locked quad panel.

## USB Link tests
Для каждого кабеля и USB-порта фиксировать тип, результат USB Test, стабильность сессии и разрывы в течение занятия.

## Capture tests
Выбор окна, закрытие, смена разрешения, сворачивание, полноэкранный режим, чёрный кадр и восстановление после остановки frame stream.

## OSD / effects
Проверить настройку, сохранение и загрузку OSD; каждый эффект и комбинации; независимые и общий выключатели. Эффекты не должны вызывать NO SIGNAL.

## Performance
Измерять capture FPS, OpenXR frame rate, end-to-end latency, CPU/GPU, shader time, память, температуру и стабильность USB Link. Численные критерии определяются после прототипа на целевом ноутбуке.
