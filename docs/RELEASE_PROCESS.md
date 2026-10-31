# Процесс выпуска версий

Используется Semantic Versioning. До первой стабильной версии — alpha/beta.

## CI/CD

GitHub Actions на Windows x64:
1. Checkout.
2. Кэш vcpkg binary packages.
3. Загрузка pinned vcpkg из baseline vcpkg.json.
4. vcpkg install всех manifest-зависимостей.
5. CMake configure.
6. MSVC build.
7. CTest.
8. ZIP packaging.
9. Upload artifact.
10. Для pre-release/* — публикация GitHub Pre-release.

Push Git-тега вида v* создаёт GitHub Release и прикрепляет ZIP. Перед стабильным релизом должны быть пройдены hardware tests с Quest 3 и проверены лицензии зависимостей.

CI подтверждает компиляцию и unit tests, но не подтверждает работу Quest Link или Passthrough: они проверяются на физическом оборудовании.
