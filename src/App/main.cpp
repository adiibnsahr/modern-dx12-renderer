#include "Platform/Win32Window.hpp"
#include "RHI/D3DUtil.hpp"
#include "RHI/GraphicsDevice.hpp"
#include "RHI/CommandQueue.hpp"
#include "RHI/CommandContext.hpp"
#include "RHI/FrameResource.hpp"
#include "RHI/SwapChain.hpp"
#include "Renderer/CubeRenderer.hpp"

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
    RenderEngine::Renderer::CubeRenderer cubeRenderer;

    try
    {
        if (!graphicsDevice.Initialize())
        {
            std::wprintf(L"[Main] Gagal membuat ID3D12Device (hardware ataupun WARP).\n");
            return -1;
        }

        if (graphicsDevice.MaxShaderModel() < D3D_SHADER_MODEL_6_0)
        {
            std::wprintf(L"GPU belum mendukung shader 6.0.\n");
            return -1;
        }

        commandQueue.Initialize(graphicsDevice.Device());

        swapChain.Initialize(
            graphicsDevice.Device(), graphicsDevice.Factory(), commandQueue.Get(),
            window.Handle(), window.Width(), window.Height()
        );

        for (FrameResource& frame : frames) frame.Initialize(graphicsDevice.Device());

        commandContext.Initialize(graphicsDevice.Device(), frames[0].CommandAllocator.Get());

        commandContext.Reset(frames[0].CommandAllocator.Get());
        cubeRenderer.Initialize(graphicsDevice.Device(), commandContext.List());
        commandContext.Close();
        commandQueue.ExecuteCommandList(commandContext.List());
        commandQueue.Flush();
    }
    catch(const DxException& e)
    {
        std::wprintf(L"[Main] DxException: %hs\n", e.what());
        return -1;
    }
    catch(const std::exception& e)
    {
        std::wprintf(L"[Main] Expection: %hs\n", e.what());
        return -1;
    }

    window.SetTitle(L"Render Engine | " + graphicsDevice.DescribeForTitleBar());

    constexpr float kClearColor[4] = { 0.05f, 0.05f, 0.08f, 1.0f };

    LARGE_INTEGER perfFreq{};
    QueryPerformanceFrequency(&perfFreq);

    LARGE_INTEGER appStartTime{};
    QueryPerformanceFrequency(&appStartTime);

    LARGE_INTEGER lastFpsTime = appStartTime;
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

        const D3D12_VIEWPORT viewport{ 0.0f, 0.0f, static_cast<float>(lastWidth), static_cast<float>(lastHeight), 0.0f, 1.0f };
        const D3D12_RECT scissorRect{ 0, 0, static_cast<LONG>(lastWidth), static_cast<LONG>(lastHeight) };
        commandContext.List()->RSSetViewports(1, &viewport);
        commandContext.List()->RSSetScissorRects(1, &scissorRect);

        LARGE_INTEGER now{};
        QueryPerformanceCounter(&now);
        const float aspectRatio = static_cast<float>(lastWidth) / static_cast<float>(lastHeight);
        const float totalTimeSeconds = static_cast<float>(
            static_cast<double>(now.QuadPart - appStartTime.QuadPart) / static_cast<double>(perfFreq.QuadPart)
        );

        cubeRenderer.Update(frameIndex, aspectRatio, totalTimeSeconds);
        cubeRenderer.Draw(commandContext.List(), frameIndex);

        commandContext.TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

        commandContext.Close();
        commandQueue.ExecuteCommandList(commandContext.List());

        swapChain.Present();
        frame.FenceValue = commandQueue.Signal();

        ++framesSinceLastFpsUpdate;
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