cmake_minimum_required(VERSION 4.3)
project(ModernD3D12Renderer LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR
        "Project ini wajib menggunakan Windows 11 + DirectX 12. "
        "Konfigurasi CMake dibatalkan karena host bukan Windows."
    )
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

include(cmake/Dependencies.cmake)

add_executable(App
    src/App/main.cpp
    src/Platform/Win32Window.hpp
    src/Platform/Win32Window.cpp
    src/RHI/D3DUtil.hpp
    src/RHI/GraphicsDevice.hpp
    src/RHI/GraphicsDevice.cpp
)

target_include_directories(App PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(App PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

target_compile_options(App PRIVATE /W4)

target_link_libraries(App PRIVATE
    d3d12
    dxgi
    DirectX-Headers
    DirectX-Guids
    DirectXMath
)

if (DEFINED agility_sdk_SOURCE_DIR)
    target_include_directories(App PRIVATE
        ${agility_sdk_SOURCE_DIR}/build/native/include
    )
    add_custom_command(TARGET App POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:App>/D3D12"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${agility_sdk_SOURCE_DIR}/build/native/bin/x64/D3D12Core.dll"
            "$<TARGET_FILE_DIR:App>/D3D12/D3D12Core.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${agility_sdk_SOURCE_DIR}/build/native/bin/x64/d3d12SDKLayers.dll"
            "$<TARGET_FILE_DIR:App>/D3D12/d3d12SDKLayers.dll"
        COMMENT "Copy runtime Agility SDK ke folder D3D12/ di App.exe"
    )
endif()