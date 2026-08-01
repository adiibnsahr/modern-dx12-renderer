#include "EditorUI.hpp"

#include "Platform/Win32Window.hpp"
#include "RHI/D3DUtil.hpp"
#include "RHI/GraphicsDevice.hpp"
#include "Scene/Camera.hpp"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#include <string>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace
{
    constexpr UINT kSrvHeapSize = 64;

    std::string WideToUtf8(const std::wstring& wide)
    {
        if (wide.empty()) return {};

        const int sizeNeeded = WideCharToMultiByte(
            CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()),
            nullptr, 0, nullptr, nullptr
        );

        std::string result(static_cast<size_t>(sizeNeeded), '\0');
        WideCharToMultiByte(
            CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()),
            result.data(), sizeNeeded, nullptr, nullptr
        );

        return result;
    }
}

namespace RenderEngine::Editor
{
    void EditorUI::Initialize(Win32Window& window, ID3D12Device* device, ID3D12CommandQueue* commandQueue,
                              UINT numFramesInFlight, DXGI_FORMAT rtvFormat)
    {
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{};
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.NumDescriptors = kSrvHeapSize;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));

        m_srvAllocator.Initialize(device, m_srvHeap.Get());

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsLight();

        ImGui_ImplWin32_Init(window.Handle());

        ImGui_ImplDX12_InitInfo initInfo{};
        initInfo.Device = device;
        initInfo.CommandQueue = commandQueue;
        initInfo.NumFramesInFlight = static_cast<int>(numFramesInFlight);
        initInfo.RTVFormat = rtvFormat;
        initInfo.DSVFormat = DXGI_FORMAT_UNKNOWN;
        initInfo.SrvDescriptorHeap = m_srvHeap.Get();
        initInfo.UserData = &m_srvAllocator;
        initInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info,
                                           D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle,
                                           D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle)
        {
            static_cast<RenderEngine::RHI::DescriptorHeapAllocator*>(info->UserData)->Alloc(outCpuHandle, outGpuHandle);
        };
        initInfo.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* info,
                                           D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
                                           D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
        {
            static_cast<RenderEngine::RHI::DescriptorHeapAllocator*>(info->UserData)->Free(cpuHandle, gpuHandle);
        };

        ImGui_ImplDX12_Init(&initInfo);

        window.SetMessageHook(ImGui_ImplWin32_WndProcHandler);
    }

    void EditorUI::Shutdown()
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void EditorUI::NewFrame()
    {
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void EditorUI::BuildDebugPanel(const GraphicsDevice& graphicsDevice, RenderEngine::Scene::Camera& camera,
                                   bool& wireframeEnabled, float fps)
    {
        if (ImGui::Begin("Render Engine Debug"))
        {
            ImGui::Text("FPS: %.1f (%.2f ms/frame)", fps, fps > 0.0f ? 1000.0f / fps : 0.0f);

            ImGui::Separator();
            ImGui::Text("Direct3D Feature Level: %s", GraphicsDevice::FeatureLevelToString(graphicsDevice.FeatureLevel()));
            ImGui::Text("Adapter: %s", WideToUtf8(graphicsDevice.AdapterDescription()).c_str());
            ImGui::Text("Backend: %s", graphicsDevice.IsWarp() ? "WARP (Software)" : "Hardware");

            ImGui::Separator();
            ImGui::Checkbox("Wireframe", &wireframeEnabled);

            if (ImGui::CollapsingHeader("Camera"))
            {
                float forceStrength = camera.ForceStrength();
                if (ImGui::SliderFloat("Force Strength", &forceStrength, 5.0f, 100.0f, "%.3f"))
                {
                    camera.SetForceStrength(forceStrength);
                }

                float damping = camera.DampingPerSecond();
                if (ImGui::SliderFloat("Damping per second", &damping, 0.001f, 0.5f, "%.3f"))
                {
                    camera.SetDampingPerSecond(damping);
                }
            }
        }

        ImGui::End();
    }

    void EditorUI::Render(ID3D12GraphicsCommandList* commandList)
    {
        ImGui::Render();

        ID3D12DescriptorHeap* heaps[] = { m_srvHeap.Get() };
        commandList->SetDescriptorHeaps(1, heaps);

        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
    }
}