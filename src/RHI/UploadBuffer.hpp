#pragma once

#include "D3DUtil.hpp"

#include <directx/d3dx12.h>
#include <d3d12.h>
#include <wrl/client.h>
#include <cstring>

namespace RenderEngine::RHI
{
    template <typename T>
    class UploadBuffer
    {
    public:
        UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer)
            : m_isConstantBuffer(isConstantBuffer)
        {
            m_elementByteSize = isConstantBuffer
                ? CalcConstantBufferByteSize(static_cast<UINT>(sizeof(T)))
                : static_cast<UINT>(sizeof(T));

            const CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
            const CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(static_cast<UINT64>(m_elementByteSize) * elementCount);

            ThrowIfFailed(device->CreateCommittedResource(
                &uploadHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_resource)
            ));

            ThrowIfFailed(m_resource->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData)));
        }

        ~UploadBuffer()
        {
            if (m_resource) m_resource->Unmap(0, nullptr);

            m_mappedData = nullptr;
        }

        UploadBuffer(const UploadBuffer&) = delete;
        UploadBuffer& operator=(const UploadBuffer&) = delete;

        [[nodiscard]] ID3D12Resource* Resource() const { return m_resource.Get(); }

        void CopyData(int elementIndex, const T& data)
        {
            memcpy(m_mappedData + static_cast<size_t>(elementIndex) * m_elementByteSize, &data, sizeof(T));
        }

    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
        BYTE* m_mappedData = nullptr;
        UINT m_elementByteSize = 0;
        bool m_isConstantBuffer = false;
    };
}