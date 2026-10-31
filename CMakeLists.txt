cmake_minimum_required(VERSION 3.25)

cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(FPVSignalTrainer VERSION 0.2.0 LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "FPV Signal Trainer currently targets Windows only.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(FPV_BUILD_TESTS "Build unit tests" ON)

find_package(OpenXR CONFIG REQUIRED)

add_executable(FPVSignalTrainer WIN32
    src/main.cpp
    src/xr/openxr_display.cpp
)
set_property(TARGET FPVSignalTrainer PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)
target_include_directories(FPVSignalTrainer PRIVATE src)
target_compile_definitions(FPVSignalTrainer PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    XR_USE_PLATFORM_WIN32
    XR_USE_GRAPHICS_API_D3D11
)
target_link_libraries(FPVSignalTrainer PRIVATE
    OpenXR::openxr_loader
    d3d11
    dxgi
    d3dcompiler
    user32
    gdi32
    advapi32
)

if(MSVC)
    target_compile_options(FPVSignalTrainer PRIVATE /W4 /permissive- /EHsc)
else()
    target_compile_options(FPVSignalTrainer PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS FPVSignalTrainer RUNTIME DESTINATION .)

if(FPV_BUILD_TESTS)
    enable_testing()
    add_executable(fpv_core_tests tests/core_tests.cpp)
    set_property(TARGET fpv_core_tests PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
    target_include_directories(fpv_core_tests PRIVATE src)
    target_compile_features(fpv_core_tests PRIVATE cxx_std_20)
    add_test(NAME fpv_core_tests COMMAND fpv_core_tests)
endif()
