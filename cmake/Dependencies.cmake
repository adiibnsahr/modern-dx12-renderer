include(FetchContent)

FetchContent_Declare(
    directx_headers
    GIT_REPOSITORY https://github.com/microsoft/DirectX-Headers.git
    GIT_TAG v1.619.4
)

FetchContent_Declare(
    directxmath
    GIT_REPOSITORY https://github.com/microsoft/DirectXMath.git
    GIT_TAG jun2026
)

FetchContent_Declare(
    agility_sdk
    URL https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.619.4
    URL_HASH SHA256=D30F756CE05BB4B7705FC1B04A5DED32ED62F2C2A2B392AE8D3318181395C8BC
)

FetchContent_MakeAvailable(
    directx_headers
    directxmath
    agility_sdk
)

