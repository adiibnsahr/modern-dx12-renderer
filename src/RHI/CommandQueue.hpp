#pragma once

#include "D3DUtil.hpp"

#include <d3d12.h>
#include <wrl/client.h>

class CommandQueue
{
public:
    CommandQueue() = default;
    ~CommandQueue();

    CommandQueue(const CommandQueue&) = delete;
    CommandQueue& operator=(const CommandQueue&) = delete;

    void Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);

    void ExecuteCommandList(ID3D12CommandList* commandList);

    [[nodiscard]] UINT64 Signal();

    [[nodiscard]] bool IsFenceComplete(UINT64 fenceValue) const;
    void WaitForFenceValue(UINT64 fenceValue);

    void Flush();

    [[nodiscard]] ID3D12CommandQueue* Get() const { return m_queue.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_queue;
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    HANDLE m_fenceEvent = nullptr;
    UINT64 m_nextFenceValue = 1;
};