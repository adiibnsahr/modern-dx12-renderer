#include "GraphicsDevice.hpp"

#include <cstdio>

namespace
{
    constexpr D3D_FEATURE_LEVEL kFeatureLevel[] = {
        D3D_FEATURE_LEVEL_12_2,
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_0,
    };
}

void GraphicsDevice::CreateFactory()
{
#if defined(_DEBUG)
    HRESULT hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_factory));
    if (FAILED(hr))
    {
        std::wprintf(L"[GraphicsDevice] DXGI_CREATE_FACTORY_DEBUG gagal (kemungkinan "
                     L"Windows 'Graphics Tools' optional feature belum terinstall). "
                     L"Lanjut tanpa DXGI debug factory.\n");

        ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(&m_factory)));
    }
#else
    ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(&m_factory)));
#endif
}

void GraphicsDevice::EnableDebugLayerIfNeeded()
{
#if defined(_DEBUG)
    Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();
        m_debugLayerEnabled = true;

        Microsoft::WRL::ComPtr<ID3D12Debug1> debugController1;
        if (SUCCEEDED(debugController.As(&debugController1)))
        {
            debugController1->SetEnableGPUBasedValidation(TRUE);
        }
    }
    else
    {
        std::wprintf(L"[GraphicsDevice] Peringatan: D3D12GetDebugInterface gagal, lanjut "
                     L"tanpa Debug Layer (kemungkinan Windows 'Graphics Tools' optional "
                     L"feature belum terinstall).\n");
    }
#endif
}

bool GraphicsDevice::TryCreateDeviceOnAdapter(const Microsoft::WRL::ComPtr<IDXGIAdapter1> &adapter)
{
    for (D3D_FEATURE_LEVEL level : kFeatureLevel)
    {
        Microsoft::WRL::ComPtr<ID3D12Device> device;
        HRESULT hr = D3D12CreateDevice(adapter.Get(), level, IID_PPV_ARGS(&device));

        if (SUCCEEDED(hr))
        {
            m_device = device;
            m_featureLevel = level;
            m_adapter = adapter;
            return true;
        }
    }

    return false;
}

bool GraphicsDevice::CreateWithWarpFallback()
{
    Microsoft::WRL::ComPtr<IDXGIAdapter1> warpAdapter;
    ThrowIfFailed(m_factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

    if (!TryCreateDeviceOnAdapter(warpAdapter))
    {
        return false;
    }

    m_usingWarp = true;
    return true;
}

bool GraphicsDevice::Initialize()
{
    EnableDebugLayerIfNeeded();
    CreateFactory();

    bool created = false;
    for (UINT i = 0;; ++i)
    {
        Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
        HRESULT hr = m_factory->EnumAdapterByGpuPreference(
            i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)
        );

        if (hr == DXGI_ERROR_NOT_FOUND) break;
        ThrowIfFailed(hr);

        DXGI_ADAPTER_DESC1 desc{};
        adapter->GetDesc1(&desc);
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

        if (TryCreateDeviceOnAdapter(adapter))
        {
            created = true;
            break;
        }
    }

    if (!created)
    {
        std::wprintf(L"[GraphicsDevice] Tidak ada adapter hardware yang lolos Feature Level 11_0 ke "
                     L"atas, fallback ke WARP");
        
        created = CreateWithWarpFallback();
    }

    if (!created) return false;

    DXGI_ADAPTER_DESC1 desc{};
    m_adapter->GetDesc1(&desc);
    m_adapterDescription = desc.Description;

    std::wprintf(L"[GraphicDevice] Device dibuat: %ls | Feature Level 0x%04X | %ls\n",
                 m_adapterDescription.c_str(),
                static_cast<unsigned>(m_featureLevel),
                m_usingWarp ? L"WARP (Software)" : L"Hardware");

    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel{};
    shaderModel.HighestShaderModel = D3D_SHADER_MODEL_6_7;
    if (SUCCEEDED(m_device->CheckFeatureSupport(
            D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)
    )))
    {
        m_maxShaderModel = shaderModel.HighestShaderModel;
    }
    std::wprintf(L"[GraphicsDevice] Shader Model tertinggi yang didukung: 0x%x "
                 L"(DXC butuh setidaknya Shader Model 6.0 = 0x60)\n",
                 static_cast<unsigned>(m_maxShaderModel));
    
    return true;
}

std::wstring GraphicsDevice::DescribeForTitleBar() const
{
    const wchar_t* levelStr = L"?";
    switch (m_featureLevel)
    {
        case D3D_FEATURE_LEVEL_12_2: levelStr = L"12_2"; break;
        case D3D_FEATURE_LEVEL_12_1: levelStr = L"12_1"; break;
        case D3D_FEATURE_LEVEL_12_0: levelStr = L"12_0"; break;
        case D3D_FEATURE_LEVEL_11_0: levelStr = L"11_0"; break;
        default: break;
    }

    return m_adapterDescription + L" | FeatureLevel " + levelStr + (m_usingWarp ? L" | WARP" : L" | Hardware");
}
