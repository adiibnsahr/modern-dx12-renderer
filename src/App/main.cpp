#include "Platform/Win32Window.hpp"
#include "RHI/D3DUtil.hpp"
#include "RHI/GraphicsDevice.hpp"
#include "RHI/CommandQueue.hpp"
#include "RHI/CommandContext.hpp"
#include "RHI/FrameResource.hpp"
#include "RHI/SwapChain.hpp"

#include <Windows.h>
#include <cstdio>
#include <array>

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
    CommandQueue commandQueue;
    SwapChain swapChain;
    std::array<FrameResource, SwapChain::kBackBufferCount> frames;
    CommandContext commandContext;

    try
    {
        if (!graphicsDevice.Initialize())
        {
            std::wprintf(L"[Main] Gagal membuat ID3D12Device (hardware ataupun WARP).\n");
            return -1;
        }

        commandQueue.Initialize(graphicsDevice.Device());

        swapChain.Initialize(
            graphicsDevice.Device(), graphicsDevice.Factory(), commandQueue.Get(),
            window.Handle(), window.Width(), window.Height()
        );

        for (FrameResource& frame : frames) frame.Initialize(graphicsDevice.Device());

        commandContext.Initialize(graphicsDevice.Device(), frames[0].CommandAllocator.Get());
    }
    catch(const DxException& e)
    {
        std::wprintf(L"[Main] DxException: %hs\n", e.what());
        return -1;
    }

    window.SetTitle(L"Render Engine | " + graphicsDevice.DescribeForTitleBar());

    constexpr float kClearColor[4] = { 0.05f, 0.05f, 0.08f, 1.0f };

    LARGE_INTEGER perfFreq{};
    LARGE_INTEGER lastFpsTime{};
    QueryPerformanceFrequency(&perfFreq);
    QueryPerformanceCounter(&lastFpsTime);
    int framesSinceLastFpsUpdate = 0;

    UINT lastWidth = window.Width();
    UINT lastHeight = window.Height();

    while (window.ProcessMessages())
    {
        if (window.IsMinimized())
        {
            Sleep(16);
            continue;
        }

        if (window.Width() != lastWidth || window.Height() != lastHeight)
        {
            lastWidth = window.Width();
            lastHeight = window.Height();
            swapChain.Resize(graphicsDevice.Device(), commandQueue, lastWidth, lastHeight);
        }

        swapChain.WaitForFrameLatency();

        const UINT frameIndex = swapChain.CurrentBackBufferIndex();
        FrameResource& frame = frames[frameIndex];

        commandQueue.WaitForFenceValue(frame.FenceValue);

        commandContext.Reset(frame.CommandAllocator.Get());

        ID3D12Resource* backBuffer = swapChain.CurrentBackBuffer();
        commandContext.TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        const D3D12_CPU_DESCRIPTOR_HANDLE rtv = swapChain.CurrentBackBufferRtv();
        commandContext.List()->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
        commandContext.List()->ClearRenderTargetView(rtv, kClearColor, 0, nullptr);

        commandContext.TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

        commandContext.Close();
        commandQueue.ExecuteCommandList(commandContext.List());

        swapChain.Present();
        frame.FenceValue = commandQueue.Signal();

        ++framesSinceLastFpsUpdate;
        LARGE_INTEGER now{};
        QueryPerformanceCounter(&now);
        const double elapsedSec = static_cast<double>(now.QuadPart - lastFpsTime.QuadPart) / static_cast<double>(perfFreq.QuadPart);

        if (elapsedSec >= 0.5)
        {
            const double fps = framesSinceLastFpsUpdate / elapsedSec;
            wchar_t fpsText[64];
            swprintf_s(fpsText, L"%.1f FPS", fps);
            window.SetTitle(L"Render Engine | " + graphicsDevice.DescribeForTitleBar() + L" | " + fpsText);
            framesSinceLastFpsUpdate = 0;
            lastFpsTime = now;
        }
    }

    commandQueue.Flush();

    std::wprintf(L"============= Render Engine keluar dengan normal =============\n");
    return 0;
}