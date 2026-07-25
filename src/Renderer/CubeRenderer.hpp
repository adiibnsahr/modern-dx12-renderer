#pragma once

#include "RHI/D3DUtil.hpp"
#include "RHI/UploadBuffer.hpp"
#include "RHI/SwapChain.hpp"

#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl/client.h>
#include <array>
#include <memory>

struct ObjectConstansts
{
    DirectX::XMFLOAT4X4 WorldViewProjection;
};

namespace RenderEngine::Renderer
{
    class CubeRenderer
    {
    public:
        CubeRenderer() = default;
        ~CubeRenderer() = default;

        CubeRenderer(const CubeRenderer&) = delete;
        CubeRenderer& operator=(const CubeRenderer&) = delete;

        void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* uploadCommandList);

        void Update(UINT frameIndex, float aspectRatio, float totalTimeSeconds);

        void Draw(ID3D12GraphicsCommandList* commandList, UINT frameIndex);

    private:
        void BuildRootSignature(ID3D12Device* device);
        void BuildPipelineState(ID3D12Device* device);
        void BuildCubeGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* uploadCommandList);

        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;

        Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBufferGPU;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBufferUploader;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBufferGPU;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBufferUploader;

        D3D12_VERTEX_BUFFER_VIEW m_vbv{};
        D3D12_INDEX_BUFFER_VIEW m_ibv{};
        UINT m_indexCount = 0;

        std::array<std::unique_ptr<RenderEngine::RHI::UploadBuffer<ObjectConstansts>>, SwapChain::kBackBufferCount> m_frameConstantBuffers;
    };
}