#include "CommandContext.hpp"

void CommandContext::Initialize(ID3D12Device* device, ID3D12CommandAllocator* initialAllocator)
{
    ThrowIfFailed(device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        initialAllocator,
        nullptr,
        IID_PPV_ARGS(&m_commandList)
    ));

    ThrowIfFailed(m_commandList->Close());
}

void CommandContext::Reset(ID3D12CommandAllocator* allocator, ID3D12PipelineState* pso)
{
    ThrowIfFailed(allocator->Reset());
    ThrowIfFailed(m_commandList->Reset(allocator, pso));
}

void CommandContext::Close()
{
    ThrowIfFailed(m_commandList->Close());
}

void CommandContext::TransitionBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = resource;
    barrier.Transition.StateBefore = before;
    barrier.Transition.StateAfter = after;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_commandList->ResourceBarrier(1, &barrier);
}