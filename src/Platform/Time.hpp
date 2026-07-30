#pragma once

#include <cstdint>

// Diadaptasi dari pola yang digunakan GameTimer.h dari buku Frank Luna
// Introduction Game Programming DirectX 12. Menggunakan cstdint untuk
// standarisasi C++ (karena Frank Luna menggunakan standarisari MSVC).
namespace RenderEngine::Platform
{
    class Time
    {
    public:
        Time();

        [[nodiscard]] float TotalTime() const;
        [[nodiscard]] float DeltaTime() const { return static_cast<float>(m_deltaTime); }

        void Reset();
        void Start();
        void Stop();
        void Tick();

    private:
        double m_secondsPerCount = 0.0;
        double m_deltaTime = -1.0;

        int64_t m_baseTime = 0;
        int64_t m_pausedTime = 0;
        int64_t m_stopTime = 0;
        int64_t m_previousTime = 0;
        int64_t m_currentTime = 0;

        bool m_stopped = false;
    };
}