#pragma once

#include "D3DUtil.hpp"

#include <d3d12.h>
#include <wrl/client.h>

struct FrameResource
{
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;
    UINT64 FenceValue = 0;

    void Initialize(ID3D12Device* device)
    {
        ThrowIfFailed(device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&CommandAllocator)
        ));
    }
};