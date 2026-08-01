#include "Win32Window.hpp"

Win32Window* Win32Window::s_instance = nullptr;

Win32Window::~Win32Window()
{
    if (m_hwnd)
    {
        DestroyWindow(m_hwnd);
    }

    if (m_hInstance)
    {
        UnregisterClassW(kWindowClassName, m_hInstance);
    }

    if (s_instance == this)
    {
        s_instance = nullptr;
    }
}

bool Win32Window::Initialize(HINSTANCE hInstance, int width, int height, const std::wstring& title)
{
    m_hInstance = hInstance;
    m_width = static_cast<uint32_t>(width);
    m_height = static_cast<uint32_t>(height);
    s_instance = this;

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszClassName = kWindowClassName;

    if (!RegisterClassExW(&wc))
    {
        return false;
    }

    RECT rect { 0, 0, width, height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    m_hwnd = CreateWindowExW(
        0,
        kWindowClassName,
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!m_hwnd)
    {
        return false;
    }

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
    return true;
}

bool Win32Window::ProcessMessages()
{
    MSG msg{};
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            m_shouldClose = true;
        }
        
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return !m_shouldClose;
}

void Win32Window::SetTitle(const std::wstring& title)
{
    SetWindowTextW(m_hwnd, title.c_str());
}

LRESULT CALLBACK Win32Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (s_instance)
    {
        return s_instance->HandleMessage(hwnd, msg, wParam, lParam);
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT Win32Window::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (m_messageHook && m_messageHook(hwnd, msg, wParam, lParam)) return true;

    switch (msg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_SIZE:
            m_width = LOWORD(lParam);
            m_height = HIWORD(lParam);
            m_minimized = (wParam == SIZE_MINIMIZED);
            return 0;

        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}