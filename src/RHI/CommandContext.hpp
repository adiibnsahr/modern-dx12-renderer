#pragma once

#include "D3DUtil.hpp"

#include <d3d12.h>
#include <wrl/client.h>

class CommandContext
{
public:
    CommandContext() = default;
    ~CommandContext() = default;

    CommandContext(const CommandContext&) = delete;
    CommandContext& operator=(const CommandContext&) = delete;

    void Initialize(ID3D12Device* device, ID3D12CommandAllocator* initialAllocator);

    void Reset(ID3D12CommandAllocator* allocator, ID3D12PipelineState* pso = nullptr);
    void Close();

    void TransitionBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

    [[nodiscard]] ID3D12GraphicsCommandList* List() const { return m_commandList.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
};