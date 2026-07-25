#include "CubeRenderer.hpp"

#include <directx/d3dx12.h>
#include <DirectXMath.h>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

using namespace DirectX;

namespace
{
    struct Vertex
    {
        XMFLOAT3 Position;
        XMFLOAT4 Color;
    };

    std::filesystem::path GetExecutableDirectory()
    {
        wchar_t buffer[MAX_PATH]{};
        GetModuleFileName(nullptr, buffer, MAX_PATH);
        return std::filesystem::path(buffer).parent_path();
    }

    std::vector<char> ReadFileBytes(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file)
        {
            throw std::runtime_error(
                "Gagal membuka file shader terkompilasi: " + path.string() +
                ", pastikan target CompileShaders sudah berjalan."
            );
        }

        const std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(static_cast<size_t>(size));
        if (!file.read(buffer.data(), size))
        {
            throw std::runtime_error("Gagal membaca file shader terkompilasi: " + path.string());
        }

        return buffer;
    }
}

namespace RenderEngine::Renderer
{
    void CubeRenderer::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* uploadCommandList)
    {
        BuildRootSignature(device);
        BuildPipelineState(device);
        BuildCubeGeometry(device, uploadCommandList);

        for (auto& cb : m_frameConstantBuffers)
        {
            cb = std::make_unique<RenderEngine::RHI::UploadBuffer<ObjectConstansts>>(device, 1, true);
        }
    }

    void CubeRenderer::BuildRootSignature(ID3D12Device* device)
    {
        D3D12_ROOT_PARAMETER rootParam{};
        rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParam.Descriptor.ShaderRegister = 0;
        rootParam.Descriptor.RegisterSpace = 0;
        rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
        rootSigDesc.NumParameters = 1;
        rootSigDesc.pParameters = &rootParam;
        rootSigDesc.NumStaticSamplers = 0;
        rootSigDesc.pStaticSamplers = nullptr;
        rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSignature;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
        const HRESULT hr = D3D12SerializeRootSignature(
            &rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSignature, &errorBlob
        );

        if (FAILED(hr))
        {
            if (errorBlob) OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
            
            ThrowIfFailed(hr);
        }

        ThrowIfFailed(device->CreateRootSignature(
            0, serializedRootSignature->GetBufferPointer(), serializedRootSignature->GetBufferSize(),
            IID_PPV_ARGS(&m_rootSignature)
        ));
    }

    void CubeRenderer::BuildPipelineState(ID3D12Device* device)
    {
        const std::filesystem::path shaderDir = GetExecutableDirectory() / L"Shaders";
        const std::vector<char> vsBytecode = ReadFileBytes(shaderDir / L"CubeVS.cso");
        const std::vector<char> psBytecode = ReadFileBytes(shaderDir / L"CubePS.cso");

        const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
        psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
        psoDesc.pRootSignature = m_rootSignature.Get();
        psoDesc.VS = { vsBytecode.data(), vsBytecode.size() };
        psoDesc.PS = { psBytecode.data(), psBytecode.size() };
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = SwapChain::kBackBufferFormat;
        psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.SampleDesc.Quality = 0;

        ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pso)));
    }

    void CubeRenderer::BuildCubeGeometry(ID3D12Device* device, ID3D12GraphicsCommandList* uploadCommandList)
    {
        const std::array<Vertex, 8> vertices = {
            Vertex{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
            Vertex{ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
            Vertex{ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
            Vertex{ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
            Vertex{ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
            Vertex{ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
            Vertex{ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
            Vertex{ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
        };

        const std::array<std::uint16_t, 36> indices = {
            0, 1, 2, 0, 2, 3,
            4, 6, 5, 4, 7, 6,
            4, 5, 1, 4, 1, 0,
            3, 2, 6, 3, 6, 7,
            1, 5, 6, 1, 6, 2,
            4, 0, 3, 4, 3, 7,
        };

        m_indexCount = static_cast<UINT>(indices.size());

        const UINT64 vbByteSize =  vertices.size() * sizeof(Vertex);
        const UINT64 ibByteSize = indices.size() * sizeof(std::uint16_t);

        m_vertexBufferGPU = CreateDefaultBuffer(device, uploadCommandList, vertices.data(), vbByteSize, m_vertexBufferUploader);
        m_indexBufferGPU = CreateDefaultBuffer(device, uploadCommandList, indices.data(), ibByteSize, m_indexBufferUploader);

        m_vbv.BufferLocation = m_vertexBufferGPU->GetGPUVirtualAddress();
        m_vbv.StrideInBytes = static_cast<UINT>(sizeof(Vertex));
        m_vbv.SizeInBytes = static_cast<UINT>(vbByteSize);

        m_ibv.BufferLocation = m_indexBufferGPU->GetGPUVirtualAddress();
        m_ibv.Format = DXGI_FORMAT_R16_UINT;
        m_ibv.SizeInBytes = static_cast<UINT>(ibByteSize);
    }

    void CubeRenderer::Update(UINT frameIndex, float aspectRatio, float totalTimeSeconds)
    {
        const XMMATRIX world = DirectX::XMMatrixRotationY(totalTimeSeconds);
        
        const XMVECTOR eyePos = DirectX::XMVectorSet(0.0f, 1.5f, -5.0f, 1.0f);
        const XMVECTOR focusPos = DirectX::XMVectorZero();
        const XMVECTOR upDir = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        const XMMATRIX view = DirectX::XMMatrixLookAtLH(eyePos, focusPos, upDir);

        const XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(XM_PIDIV4, aspectRatio, 0.1f, 100.0f);

        const XMMATRIX worldViewProjection = world * view * proj;

        ObjectConstansts objConstants;

        DirectX::XMStoreFloat4x4(&objConstants.WorldViewProjection, DirectX::XMMatrixTranspose(worldViewProjection));

        m_frameConstantBuffers[frameIndex]->CopyData(0, objConstants);
    }

    void CubeRenderer::Draw(ID3D12GraphicsCommandList* commandList, UINT frameIndex)
    {
        commandList->SetGraphicsRootSignature(m_rootSignature.Get());
        commandList->SetPipelineState(m_pso.Get());

        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->IASetVertexBuffers(0, 1, &m_vbv);
        commandList->IASetIndexBuffer(&m_ibv);

        commandList->SetGraphicsRootConstantBufferView(
            0, m_frameConstantBuffers[frameIndex]->Resource()->GetGPUVirtualAddress()
        );

        commandList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
    }
}