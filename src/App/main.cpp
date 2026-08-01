#include "Platform/Win32Window.hpp"
#include "Platform/Time.hpp"
#include "Platform/Input.hpp"
#include "RHI/D3DUtil.hpp"
#include "RHI/GraphicsDevice.hpp"
#include "RHI/CommandQueue.hpp"
#include "RHI/CommandContext.hpp"
#include "RHI/FrameResource.hpp"
#include "RHI/SwapChain.hpp"
#include "Scene/Camera.hpp"
#include "Renderer/CubeRenderer.hpp"
#include "Editor/EditorUI.hpp"

#include <DirectXMath.h>
#include <Windows.h>
#include <cstdio>
#include <array>

using namespace DirectX;

namespace
{
    constexpr float kMouseSensitivity = 0.0025f;
    constexpr float kSprintMultiplier = 3.0f;
}

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
    RenderEngine::Scene::Camera camera;
    RenderEngine::Platform::Input input;
    RenderEngine::Platform::Time time;
    RenderEngine::Editor::EditorUI editor;

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

        editor.Initialize(window, graphicsDevice.Device(), commandQueue.Get(),
                          SwapChain::kBackBufferCount, SwapChain::kBackBufferFormat);

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

    camera.SetLens(XM_PIDIV4, static_cast<float>(window.Width()) / static_cast<float>(window.Height()), 0.1f, 100.0f);

    window.SetTitle(L"Render Engine | " + graphicsDevice.DescribeForTitleBar());

    constexpr float kClearColor[4] = { 0.05f, 0.05f, 0.08f, 1.0f };

    time.Reset();

    float lastFpsUpdateTime = 0.0f;
    int frameSinceLastFpsUpdate = 0;

    float currentFps = 0.0f;

    bool wireframeEnabled = false;

    UINT lastWidth = window.Width();
    UINT lastHeight = window.Height();

    while (window.ProcessMessages())
    {
        if (window.IsMinimized())
        {
            Sleep(16);
            continue;
        }

        time.Tick();

        if (window.Width() != lastWidth || window.Height() != lastHeight)
        {
            lastWidth = window.Width();
            lastHeight = window.Height();
            swapChain.Resize(graphicsDevice.Device(), commandQueue, lastWidth, lastHeight);
            camera.SetLens(XM_PIDIV4, static_cast<float>(window.Width()) / static_cast<float>(window.Height()), 0.1f, 100.0f);
        }

        const bool hasFocus = (GetForegroundWindow() == window.Handle());
        input.Update(window.Handle(), hasFocus);

        XMFLOAT3 moveDir { 0.0f, 0.0f, 0.0f };
        if (input.IsKeyDown('W')) moveDir.z += 1.0f;
        if (input.IsKeyDown('S')) moveDir.z -= 1.0f;
        if (input.IsKeyDown('D')) moveDir.x += 1.0f;
        if (input.IsKeyDown('A')) moveDir.x -= 1.0f;
        if (input.IsKeyDown('E')) moveDir.y += 1.0f;
        if (input.IsKeyDown('Q')) moveDir.y -= 1.0f;

        const float speedMultiplier = input.IsKeyDown(VK_SHIFT) ? kSprintMultiplier : 1.0f;
        camera.ApplyMoveForce(moveDir, speedMultiplier);

        camera.RotateY(input.MouseDeltaX() * kMouseSensitivity);
        camera.Pitch(-input.MouseDeltaY() * kMouseSensitivity);

        camera.Update(time.DeltaTime());

        editor.NewFrame();
        editor.BuildDebugPanel(graphicsDevice, camera, wireframeEnabled, currentFps);
        cubeRenderer.SetWireframe(wireframeEnabled);

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

        cubeRenderer.Update(frameIndex, camera.GetView(), camera.GetProj(), time.TotalTime());
        cubeRenderer.Draw(commandContext.List(), frameIndex);

        editor.Render(commandContext.List());

        commandContext.TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

        commandContext.Close();
        commandQueue.ExecuteCommandList(commandContext.List());

        swapChain.Present();
        frame.FenceValue = commandQueue.Signal();

        ++frameSinceLastFpsUpdate;
        const float elapsedSinceFpsUpdate = time.TotalTime() - lastFpsUpdateTime;
        if (elapsedSinceFpsUpdate >= 0.5)
        {
            currentFps = frameSinceLastFpsUpdate / elapsedSinceFpsUpdate;
            wchar_t fpsText[64];
            swprintf_s(fpsText, L"%.1f FPS", currentFps);
            window.SetTitle(L"Render Engine | " + graphicsDevice.DescribeForTitleBar() + L" | " + fpsText);
            frameSinceLastFpsUpdate = 0;
            lastFpsUpdateTime = time.TotalTime();
        }
    }

    commandQueue.Flush();
    editor.Shutdown();

    std::wprintf(L"============= Render Engine keluar dengan normal =============\n");
    return 0;
}