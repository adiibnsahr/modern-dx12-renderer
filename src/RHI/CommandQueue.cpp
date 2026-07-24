#include "CommandQueue.hpp"

CommandQueue::~CommandQueue()
{
    if (m_fenceEvent)
    {
        CloseHandle(m_fenceEvent);
    }
}

void CommandQueue::Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
{
    D3D12_COMMAND_QUEUE_DESC desc{};
    desc.Type = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_queue)));

    ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!m_fenceEvent) ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
}

void CommandQueue::ExecuteCommandList(ID3D12CommandList* commandList)
{
    ID3D12CommandList* lists[] = { commandList };
    m_queue->ExecuteCommandLists(1, lists);
}

UINT64 CommandQueue::Signal()
{
    const UINT64 valueToSignal = m_nextFenceValue;
    ThrowIfFailed(m_queue->Signal(m_fence.Get(), valueToSignal));
    return valueToSignal;
}

bool CommandQueue::IsFenceComplete(UINT64 fenceValue) const
{
    return m_fence->GetCompletedValue() >= fenceValue;
}

void CommandQueue::WaitForFenceValue(UINT64 fenceValue)
{
    if (IsFenceComplete(fenceValue)) return;
    ThrowIfFailed(m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent));
    WaitForSingleObject(m_fenceEvent, INFINITE);
}

void CommandQueue::Flush()
{
    const UINT64 value = Signal();
    WaitForFenceValue(value);
}