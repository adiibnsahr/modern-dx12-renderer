#pragma once

#include "D3DUtil.hpp"

#include <d3d12.h>
#include <vector>

namespace RenderEngine::RHI
{
    class DescriptorHeapAllocator
    {
    public:
        DescriptorHeapAllocator() = default;

        void Initialize(ID3D12Device* device, ID3D12DescriptorHeap* heap);

        void Alloc(D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle);
        void Free(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);

    private:
        D3D12_CPU_DESCRIPTOR_HANDLE m_heapStartCpu{};
        D3D12_GPU_DESCRIPTOR_HANDLE m_heapStartGpu{};
        UINT m_handleIncrement = 0;
        std::vector<int> m_freeIndices;
    };
}