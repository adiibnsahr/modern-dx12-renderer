#pragma once

#include "D3DUtil.hpp"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <string>

class GraphicsDevice
{
public:
    GraphicsDevice() = default;
    ~GraphicsDevice() = default;

    GraphicsDevice(const GraphicsDevice&) = delete;
    GraphicsDevice& operator=(const GraphicsDevice&) = delete;

    bool Initialize();

    [[nodiscard]] ID3D12Device* Device() const { return m_device.Get(); }
    [[nodiscard]] IDXGIFactory6* Factory() const { return m_factory.Get(); }
    [[nodiscard]] D3D_FEATURE_LEVEL FeatureLevel() const { return m_featureLevel; }
    [[nodiscard]] bool IsWarp() const { return m_usingWarp; }
    [[nodiscard]] const std::wstring& AdapterDescription() const { return m_adapterDescription; }

    [[nodiscard]] std::wstring DescribeForTitleBar() const;

private:
    void CreateFactory();
    void EnableDebugLayerIfNeeded();

    bool TryCreateDeviceOnAdapter(const Microsoft::WRL::ComPtr<IDXGIAdapter1> &adapter);

    bool CreateWithWarpFallback();

    Microsoft::WRL::ComPtr<IDXGIFactory6> m_factory;
    Microsoft::WRL::ComPtr<IDXGIAdapter1> m_adapter;
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;

    D3D_FEATURE_LEVEL m_featureLevel = D3D_FEATURE_LEVEL_11_0;
    bool m_usingWarp = false;
    bool m_debugLayerEnabled = false;
    std::wstring m_adapterDescription;
};