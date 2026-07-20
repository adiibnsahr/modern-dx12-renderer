#pragma once

#include <Windows.h>
#include <comdef.h>
#include <cstring>
#include <stdexcept>
#include <string>

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