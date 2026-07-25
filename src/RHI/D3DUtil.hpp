#pragma once

#include <Windows.h>
#include <comdef.h>
#include <cstring>
#include <stdexcept>
#include <string>

#include <directx/d3dx12.h>
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>

// Menggunakan pattern yang diadaptasi dari Common/d3dUtil.h pada buku
// Frank Luna, satu exception type, satu makro pemanggil.
class DxException : public std::runtime_error
{
public:
    DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber)
        : std::runtime_error(BuildMessage(hr, functionName, filename, lineNumber))
        , ErrorCode(hr)
    {}

    const HRESULT ErrorCode;

private:
    static std::string BuildMessage(
        HRESULT hr, 
        const std::wstring& functionName, 
        const std::wstring& filename, 
        int lineNumber)
    {
        _com_error err(hr);
        std::wstring msg = functionName + L" gagal di " + filename + L", baris " +
                           std::to_wstring(lineNumber) + L": " + err.ErrorMessage();

        std::string narrow;
        narrow.reserve(msg.size());
        for (wchar_t wc : msg)
        {
            narrow.push_back(static_cast<char>(wc));
        }

        return narrow;
    }
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                                    \
    {                                                                       \
        HRESULT hr__ = (x);                                                 \
        if (FAILED(hr__))                                                   \
        {                                                                   \
            const char* file__ = __FILE__;                                  \
            throw DxException(                                              \
                hr__,                                                       \
                L#x,                                                        \
                std::wstring(file__, file__ + std::strlen(file__)),         \
                __LINE__                                                    \
            );                                                              \
        }                                                                   \
    }
#endif

inline UINT CalcConstantBufferByteSize(UINT byteSize) { return (byteSize + 255) & ~255; }

inline Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* commandList,
    const void* initData,
    UINT64 byteSize,
    Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBufferOut
)
{
    Microsoft::WRL::ComPtr<ID3D12Resource> defaultBuffer;

    const CD3DX12_HEAP_PROPERTIES defaultHeapProps(D3D12_HEAP_TYPE_DEFAULT);
    const CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);

    ThrowIfFailed(device->CreateCommittedResource(
        &defaultHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
        D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&defaultBuffer)
    ));

    const CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
    ThrowIfFailed(device->CreateCommittedResource(
        &uploadHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBufferOut)
    ));

    D3D12_SUBRESOURCE_DATA subResourceData{};
    subResourceData.pData = initData;
    subResourceData.RowPitch = static_cast<LONG_PTR>(byteSize);
    subResourceData.SlicePitch = subResourceData.RowPitch;

    const CD3DX12_RESOURCE_BARRIER toCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(
        defaultBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST
    );
    commandList->ResourceBarrier(1, &toCopyDest);

    UpdateSubresources<1>(commandList, defaultBuffer.Get(), uploadBufferOut.Get(), 0, 0, 1, &subResourceData);

    const CD3DX12_RESOURCE_BARRIER toGenericRead = CD3DX12_RESOURCE_BARRIER::Transition(
        defaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ
    );
    commandList->ResourceBarrier(1, &toGenericRead);

    return defaultBuffer;
}