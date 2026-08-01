#include "DescriptorHeapAllocator.hpp"

namespace RenderEngine::RHI
{
    void DescriptorHeapAllocator::Initialize(ID3D12Device* device, ID3D12DescriptorHeap* heap)
    {
        const D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();

        m_heapStartCpu = heap->GetCPUDescriptorHandleForHeapStart();
        m_heapStartGpu = heap->GetGPUDescriptorHandleForHeapStart();
        m_handleIncrement = device->GetDescriptorHandleIncrementSize(desc.Type);

        m_freeIndices.clear();
        m_freeIndices.reserve(desc.NumDescriptors);

        for (int i = static_cast<int>(desc.NumDescriptors) - 1; i >= 0; --i)
        {
            m_freeIndices.push_back(i);
        }
    }

    void DescriptorHeapAllocator::Alloc(D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle)
    {
        if (m_freeIndices.empty())
        {
            ThrowIfFailed(E_OUTOFMEMORY);
        }

        const int index = m_freeIndices.back();
        m_freeIndices.pop_back();

        outCpuHandle->ptr = m_heapStartCpu.ptr + static_cast<SIZE_T>(index) * m_handleIncrement;
        outGpuHandle->ptr = m_heapStartGpu.ptr + static_cast<UINT64>(index) * m_handleIncrement;
    }

    void DescriptorHeapAllocator::Free(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
    {
        const int cpuIndex = static_cast<int>((cpuHandle.ptr - m_heapStartCpu.ptr) / m_handleIncrement);
        const int gpuIndex = static_cast<int>((gpuHandle.ptr - m_heapStartGpu.ptr) / m_handleIncrement);

        if (cpuIndex != gpuIndex)
        {
            ThrowIfFailed(E_INVALIDARG);
        }

        m_freeIndices.push_back(cpuIndex);
    }
}