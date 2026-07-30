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
    src/RHI/CommandQueue.hpp
    src/RHI/CommandQueue.cpp
    src/RHI/CommandContext.hpp
    src/RHI/CommandContext.cpp
    src/RHI/FrameResource.hpp
    src/RHI/SwapChain.hpp
    src/RHI/SwapChain.cpp
    src/RHI/UploadBuffer.hpp
    src/Renderer/CubeRenderer.hpp
    src/Renderer/CubeRenderer.cpp
    src/Platform/Time.hpp
    src/Platform/Time.cpp
    src/Platform/Input.hpp
    src/Platform/Input.cpp
    src/Scene/Camera.hpp
    src/Scene/Camera.cpp
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

if(DEFINED dxc_SOURCE_DIR)
    find_program(DXC_EXECUTABLE
        NAMES dxc.exe dxc
        PATHS ${dxc_SOURCE_DIR}
        PATH_SUFFIXES "build/native/bin/x64" "bin/x64"
        NO_DEFAULT_PATH
    )
    if(NOT DXC_EXECUTABLE)
        file(GLOB_RECURSE DXC_EXE_CANDIDATES "${dxc_SOURCE_DIR}/*dxc.exe")
        if(DXC_EXE_CANDIDATES)
            list(GET DXC_EXE_CANDIDATES 0 DXC_EXECUTABLE)
        endif()
    endif()
endif()

if(NOT DXC_EXECUTABLE)
    message(FATAL_ERROR
        "dxc.exe tidak ditemukan didalam pake Microsoft.Direct3D.DXC yang difetch"
    )
endif()
message(STATUS "RenderEngine: menggunakan DXC dari ${DXC_EXECUTABLE}")

set(SHADER_OUTPUT_DIR ${CMAKE_BINARY_DIR}/CompiledShaders)
file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

function(modernengine_compile_hlsl SHADER_FILE ENTRY_POINT PROFILE OUTPUT_CSO)
    add_custom_command(
        OUTPUT ${OUTPUT_CSO}
        COMMAND ${DXC_EXECUTABLE} -T ${PROFILE} -E ${ENTRY_POINT} -Fo ${OUTPUT_CSO} ${SHADER_FILE}
        DEPENDS ${SHADER_FILE}
        COMMENT "DXC: ${SHADER_FILE} [${ENTRY_POINT}, ${PROFILE}] -> ${OUTPUT_CSO}"
        VERBATIM
    )
endfunction()

set(CUBE_HLSL ${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/Cube.hlsl)
set(CUBE_VS_CSO ${SHADER_OUTPUT_DIR}/CubeVS.cso)
set(CUBE_PS_CSO ${SHADER_OUTPUT_DIR}/CubePS.cso)

modernengine_compile_hlsl(${CUBE_HLSL} VSMain vs_6_0 ${CUBE_VS_CSO})
modernengine_compile_hlsl(${CUBE_HLSL} PSMain ps_6_0 ${CUBE_PS_CSO})

add_custom_target(CompiledShaders DEPENDS ${CUBE_VS_CSO} ${CUBE_PS_CSO})
add_dependencies(App CompiledShaders)

add_custom_command(TARGET App POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:App>/Shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CUBE_VS_CSO} "$<TARGET_FILE_DIR:App>/Shaders/CubeVS.cso"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CUBE_PS_CSO} "$<TARGET_FILE_DIR:App>/Shaders/CubePS.cso"
    COMMENT "Menyalin shader yang sudah di compile ke folder Shaders/"
)

