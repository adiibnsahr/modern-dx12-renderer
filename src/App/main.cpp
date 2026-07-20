#include "Platform/Win32Window.hpp"
#include "RHI/D3DUtil.hpp"
#include "RHI/GraphicsDevice.hpp"

#include <Windows.h>
#include <cstdio>

int main()
{
    std::wprintf(L"===================== Render Engine D3D12 ====================\n");

    HINSTANCE hInstance = GetModuleHandleW(nullptr);

    Win32Window window;
    if (!window.Initialize(hInstance, 1280, 720, L"Render Engine"))
    {
        std::wprintf(L"[Main] Gagal membuat window.\n");
        return -1;
    }

    GraphicsDevice graphicsDevice;
    try
    {
        if (!graphicsDevice.Initialize())
        {
            std::wprintf(L"[Main] Gagal membuat ID3D12Device (hardware ataupun WARP).\n");
            return -1;
        }
    }
    catch(const DxException& e)
    {
        std::wprintf(L"[Main] DxException: %hs\n", e.what());
        return -1;
    }

    window.SetTitle(L"Render Engine | " + graphicsDevice.DescribeForTitleBar());

    while (window.ProcessMessages())
    {
        Sleep(window.IsMinimized() ? 16 : 1);
    }

    std::wprintf(L"============= Render Engine keluar dengan normal =============\n");
    return 0;
}