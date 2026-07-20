#pragma once

#include <Windows.h>
#include <cstdint>
#include <string>

class Win32Window
{
public:
    Win32Window() = default;
    ~Win32Window();

    Win32Window(const Win32Window&) = delete;
    Win32Window& operator=(const Win32Window&) = delete;

    bool Initialize(HINSTANCE hInstance, int width, int height, const std::wstring& title);

    [[nodiscard]] bool ProcessMessages();

    void SetTitle(const std::wstring& title);

    [[nodiscard]] HWND Handle() const { return m_hwnd; }
    [[nodiscard]] uint32_t Width() const { return m_width; }
    [[nodiscard]] uint32_t Height() const { return m_height; }
    [[nodiscard]] bool IsMinimized() const { return m_minimized; }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Adaptasi dari buku Frank Luna (menggunakan static instance pointer)
    static Win32Window* s_instance;

    HWND m_hwnd = nullptr;
    HINSTANCE m_hInstance = nullptr;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    bool m_minimized = false;
    bool m_shouldClose = false;

    static constexpr wchar_t kWindowClassName[] = L"ModernD3D12RendererClass";
};