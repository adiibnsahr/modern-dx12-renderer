#pragma once

#include "D3DUtil.hpp"
#include "CommandQueue.hpp"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <array>

class SwapChain
{
public:
    static constexpr UINT kBackBufferCount = 3;
    static constexpr DXGI_FORMAT kBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    SwapChain() = default;
    ~SwapChain();

    SwapChain(const SwapChain&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;

    void Initialize(ID3D12Device* device, IDXGIFactory6* factory, ID3D12CommandQueue* commandQueue, HWND hwnd, UINT width, UINT height);

    void Resize(ID3D12Device* device, CommandQueue& queue, UINT width, UINT height);

    void WaitForFrameLatency() const;

    void Present();

    [[nodiscard]] UINT CurrentBackBufferIndex() const;
    [[nodiscard]] ID3D12Resource* CurrentBackBuffer() const { return m_backBuffers[CurrentBackBufferIndex()].Get(); }
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferRtv() const { return m_rtvHandle[CurrentBackBufferIndex()]; }

private:
    void CreateRenderTargetViews(ID3D12Device* device);

    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, kBackBufferCount> m_backBuffers;
    std::array<D3D12_CPU_DESCRIPTOR_HANDLE, kBackBufferCount> m_rtvHandle{};
    UINT m_rtvDescriptorSize = 0;
    HANDLE m_frameLatencyWaitableObject = nullptr;
};