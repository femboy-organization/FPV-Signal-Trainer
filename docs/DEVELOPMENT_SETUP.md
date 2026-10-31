# Настройка среды разработки

Версии Unity и пакетов будут закреплены после успешного технического прототипа.

## Оборудование
- Windows 10/11 PC с совместимым GPU.
- Meta Quest 3.
- Проверенный USB-A–USB-C или USB-C–USB-C кабель.
- Клавиатура и мышь.
- FPV-симулятор для проверки захвата.

## Программное обеспечение
Предполагаемый набор:
- Unity Hub и согласованная Unity 6;
- Windows Build Support;
- Visual Studio или совместимая C# IDE;
- Meta Quest Link для Windows;
- Git;
- Git LFS — только если понадобится хранить крупные бинарные ресурсы.

В репозитории должны храниться ProjectSettings, Packages/manifest.json, Packages/packages-lock.json, Assets и .meta-файлы.

## Подготовка Quest Link
1. Установить Meta Quest Link из официального источника Meta.
2. Подключить Quest 3 кабелем.
3. Подтвердить USB Link в гарнитуре, если появится запрос.
4. Проверить запуск PCVR-сессии.
5. Выполнить USB Test и зафиксировать результат для каждого кабеля/порта.
6. Не использовать Air Link или Wi-Fi для проекта.

## Подготовка Unity
После фиксации версии:
1. Установить согласованный Unity Editor и Windows Build Support.
2. Открыть проект из репозитория.
3. Установить зафиксированные XR/OpenXR пакеты.
4. Настроить Windows Standalone target.
5. Запустить Quest Link до запуска приложения.
6. Проверить тестовую XR-сцену и head tracking.

Точный набор пакетов и настройки будут записаны после первого успешного прототипа.

## Проверка захвата
Для каждого симулятора фиксировать название/версию, оконный режим, разрешение, частоту обновления, поведение при сворачивании, смене разрешения, полноэкранном режиме и измеренную задержку.

## Git
Работать в отдельных ветках и через Pull Request. Не коммитить Library, Temp, Obj, Build, локальные настройки, секреты и персональные данные.

## Ссылки
- [Meta Quest Link](https://www.meta.com/quest/quest-link/)
- [Meta Unity documentation](https://developers.meta.com/vr/documentation/unity/)
- [Unity OpenXR Plugin](https://docs.unity3d.com/Packages/com.unity.xr.openxr@latest)
- [Windows Graphics Capture](https://learn.microsoft.com/windows/apps/develop/media-authoring-processing/screen-capture)
