include(FetchContent)

# ========================================
#  DirectX Headers
# ========================================
FetchContent_Declare(
    directx_headers
    GIT_REPOSITORY https://github.com/microsoft/DirectX-Headers.git
    GIT_TAG v1.619.4
)

# ========================================
#  DirectX Math
# ========================================
FetchContent_Declare(
    directxmath
    GIT_REPOSITORY https://github.com/microsoft/DirectXMath.git
    GIT_TAG jun2026
)

# ========================================
#  DirectX 12 NuGet
# ========================================
FetchContent_Declare(
    agility_sdk
    URL https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.619.4
    URL_HASH SHA256=D30F756CE05BB4B7705FC1B04A5DED32ED62F2C2A2B392AE8D3318181395C8BC
)

# ========================================
#  DirectX Compiler
# ========================================
FetchContent_Declare(
    dxc
    URL https://www.nuget.org/api/v2/package/Microsoft.Direct3D.DXC/1.9.2602.24
    #URL HASH menyusul
)

# ========================================
#  Imgui
# ========================================
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.8
)

FetchContent_MakeAvailable(
    directx_headers
    directxmath
    agility_sdk
    dxc
    imgui
)

