#pragma once

#include "RHI/DescriptorHeapAllocator.hpp"
#include "Scene/Camera.hpp"

#include <d3d12.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <Windows.h>

class Win32Window;
class GraphicsDevice;
class RenderEngine::Scene::Camera;

namespace RenderEngine::Editor
{
    class EditorUI
    {
    public:
        EditorUI() = default;
        ~EditorUI() = default;

        EditorUI(const EditorUI&) = delete;
        EditorUI& operator=(const EditorUI&) = delete;

        void Initialize(Win32Window& window, ID3D12Device* device, ID3D12CommandQueue* commandQueue,
                        UINT numFramesInFlight, DXGI_FORMAT rtvFormat);
        void Shutdown();
        
        void NewFrame();

        void BuildDebugPanel(const GraphicsDevice& graphicsDevice, RenderEngine::Scene::Camera& camera,
                             bool& wireframeEnabled, float fps);

        void Render(ID3D12GraphicsCommandList* commandList);
    private:
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;
        RenderEngine::RHI::DescriptorHeapAllocator m_srvAllocator;
    };
}