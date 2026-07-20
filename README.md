# Modern DirectX 12 Render Engine

Sebuah project minimalis mesin render

## Tech Stack

| Komponen | Pilihan |
|---|---|
| Bahasa | C++20 (`.hpp`/`.cpp`) |
| Graphics API | DirectX 12 (Agility SDK, Feature Level target `12_2`, minimum `11_0`) |
| Build system | CMake ≥ 4.3 (`FetchContent`, tanpa vcpkg) |
| Compiler | MSVC (Visual Studio 2026 Build Tools) |
| OS target | Windows 11 |
| Windowing | Win32 native |

## Version 0.0.1
- Windowing tanpa swapchain

**Referensi:**
- Buku Frank Luna: `d3d12book_2ed`, `Common/d3dUtil.h`, `ThrowIfFailed`
- Microsoft Sample: `D3D12HelloWorld`, `GetHardwareAdapter`, skip software adapter, iterasi melalui `EnumAdapterByGpuPreference`
- Microsoft Learn: `D3D12CreteDevice`, `CheckFeatureSupport`, `EnumWarpAdapter`

**Build dan Run**
```bash
cmake --preset windows-msvc

cmake --build build --config Debug

./build/Debug/App.exe
```