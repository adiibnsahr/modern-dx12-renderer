#include "SwapChain.hpp"

SwapChain::~SwapChain()
{
    if (m_frameLatencyWaitableObject) CloseHandle(m_frameLatencyWaitableObject);
}

void SwapChain::Initialize(ID3D12Device* device, IDXGIFactory6* factory, ID3D12CommandQueue* commandQueue, HWND hwnd, UINT width, UINT height)
{
    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.Width = width;
    desc.Height = height;
    desc.Format = kBackBufferFormat;
    desc.Stereo = FALSE;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = kBackBufferCount;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    desc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
    ThrowIfFailed(factory->CreateSwapChainForHwnd(commandQueue, hwnd, &desc, nullptr, nullptr, &swapChain1));

    ThrowIfFailed(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swapChain1.As(&m_swapChain));

    ThrowIfFailed(m_swapChain->SetMaximumFrameLatency(1));
    m_frameLatencyWaitableObject = m_swapChain->GetFrameLatencyWaitableObject();

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
    rtvHeapDesc.NumDescriptors = kBackBufferCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
    m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    CreateRenderTargetViews(device);
}

void SwapChain::CreateRenderTargetViews(ID3D12Device* device)
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < kBackBufferCount; ++i)
    {
        ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i])));
        device->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, rtvHandle);
        m_rtvHandle[i] = rtvHandle;
        rtvHandle.ptr += m_rtvDescriptorSize;
    }
}

void SwapChain::Resize(ID3D12Device* device, CommandQueue& queue, UINT width, UINT height)
{
    if (width == 0 || height == 0) return;

    queue.Flush();

    for (auto& backBuffer : m_backBuffers) backBuffer.Reset();

    DXGI_SWAP_CHAIN_DESC currentDesc{};
    ThrowIfFailed(m_swapChain->GetDesc(&currentDesc));
    ThrowIfFailed(m_swapChain->ResizeBuffers(kBackBufferCount, width, height, currentDesc.BufferDesc.Format, currentDesc.Flags));

    CreateRenderTargetViews(device);
}

void SwapChain::WaitForFrameLatency() const
{
    WaitForSingleObjectEx(m_frameLatencyWaitableObject, 1000, TRUE);
}

void SwapChain::Present()
{
    ThrowIfFailed(m_swapChain->Present(1, 0));
}

UINT SwapChain::CurrentBackBufferIndex() const
{
    return m_swapChain->GetCurrentBackBufferIndex();
}