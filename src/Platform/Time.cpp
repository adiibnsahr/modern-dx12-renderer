#include "Time.hpp"

#include <Windows.h>

namespace RenderEngine::Platform
{
    Time::Time()
    {
        int64_t countsPerSec = 0;
        QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&countsPerSec));
        m_secondsPerCount = 1.0 / static_cast<double>(countsPerSec);
    }

    float Time::TotalTime() const
    {
        if (m_stopped)
        {
            return static_cast<float>(((m_stopTime - m_pausedTime) - m_baseTime) * m_secondsPerCount);
        }

        return static_cast<float>(((m_currentTime - m_pausedTime) - m_baseTime) * m_secondsPerCount);
    }

    void Time::Reset()
    {
        int64_t currentTime = 0;
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currentTime));

        m_baseTime = currentTime;
        m_previousTime = currentTime;
        m_stopTime = 0;
        m_stopped = false;
    }

    void Time::Start()
    {
        if (m_stopped)
        {
            int64_t startTime = 0;
            QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&startTime));

            m_pausedTime += (startTime - m_stopTime);
            m_previousTime = startTime;
            m_stopTime = 0;
            m_stopped = false;
        }
    }

    void Time::Stop()
    {
        if (!m_stopped)
        {
            int64_t currentTime = 0;
            QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currentTime));

            m_stopTime = currentTime;
            m_stopped = true;
        }
    }

    void Time::Tick()
    {
        if (m_stopped)
        {
            m_deltaTime = 0.0;
            return;
        }

        int64_t currentTime = 0;
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currentTime));
        m_currentTime = currentTime;

        m_deltaTime = (m_currentTime - m_previousTime) * m_secondsPerCount;
        m_previousTime = m_currentTime;

        if (m_deltaTime < 0.0)
        {
            m_deltaTime = 0.0;
        }
    }
}