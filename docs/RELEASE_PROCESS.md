# Процесс выпуска версий

Используется Semantic Versioning. До первой стабильной версии — alpha/beta.

GitHub Actions на Windows x64:
1. Checkout.
2. Bootstrap зафиксированного vcpkg.
3. CMake configure.
4. MSVC build.
5. CTest.
6. ZIP packaging.
7. Upload artifact.

Push Git-тега вида v* создаёт GitHub Release и прикрепляет ZIP. Перед стабильным релизом должны быть пройдены hardware tests с Quest 3 и проверены лицензии зависимостей.

CI подтверждает компиляцию и unit tests, но не подтверждает работу Quest Link или Passthrough: они проверяются на физическом оборудовании.
