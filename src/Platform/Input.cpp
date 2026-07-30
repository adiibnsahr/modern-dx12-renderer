#include "Input.hpp"

namespace RenderEngine::Platform
{
    void Input::Update(HWND hwnd, bool hasFocus)
    {
        m_hasFocus = hasFocus;
        m_mouseDeltaX = 0.0f;
        m_mouseDeltaY = 0.0f;

        if (!hasFocus)
        {
            if (m_mouseCaptured)
            {
                ShowCursor(TRUE);
                m_mouseCaptured = false;
            }

            return;
        }

        const bool wantsCapture = IsKeyDown(VK_RBUTTON);

        if (wantsCapture && !m_mouseCaptured)
        {
            ShowCursor(FALSE);
            m_mouseCaptured = true;
            m_firstCapturedFrame = true;
        }
        else if (!wantsCapture && m_mouseCaptured)
        {
            ShowCursor(TRUE);
            m_mouseCaptured = false;
        }

        if (m_mouseCaptured)
        {
            RECT clientRect{};
            GetClientRect(hwnd, &clientRect);
            POINT center { (clientRect.right - clientRect.left) / 2, (clientRect.bottom - clientRect.top) / 2 };
            ClientToScreen(hwnd, &center);

            POINT cursorPos{};
            GetCursorPos(&cursorPos);

            if (!m_firstCapturedFrame)
            {
                m_mouseDeltaX = static_cast<float>(cursorPos.x - center.x);
                m_mouseDeltaY = static_cast<float>(cursorPos.y - center.y);
            }

            m_firstCapturedFrame = false;

            SetCursorPos(center.x, center.y);
        }
    }

    bool Input::IsKeyDown(int virtualKey) const
    {
        if (!m_hasFocus) return false;
        return (GetAsyncKeyState(virtualKey) & 0x8000) != 0;
    }
}