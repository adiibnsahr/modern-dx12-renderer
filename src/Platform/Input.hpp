#pragma once

#include <Windows.h>

namespace RenderEngine::Platform
{
    class Input
    {
    public:
        void Update(HWND hwnd, bool hasFocus);

        [[nodiscard]] bool IsKeyDown(int virtualKey) const;
        [[nodiscard]] float MouseDeltaX() const { return m_mouseDeltaX; }
        [[nodiscard]] float MouseDeltaY() const { return m_mouseDeltaY; }

    private:
        bool m_hasFocus = true;
        bool m_mouseCaptured = false;
        bool m_firstCapturedFrame = true;
        float m_mouseDeltaX = 0.0f;
        float m_mouseDeltaY = 0.0f;

    };
}