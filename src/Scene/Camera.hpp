#pragma once

#include <DirectXMath.h>

namespace RenderEngine::Scene
{
    class Camera
    {
    public:
        Camera();

        void SetLens(float fovY, float aspect, float nearZ, float farZ);
        void SetPosition(float x, float y, float z);

        void ApplyMoveForce(const DirectX::XMFLOAT3& direction, float speedMultiplier = 1.0f);

        void Pitch(float angleRadians);
        void RotateY(float angleRadians);

        void Update(float dt);

        [[nodiscard]] DirectX::XMMATRIX GetView() const;
        [[nodiscard]] DirectX::XMMATRIX GetProj() const;

        [[nodiscard]] float ForceStrength() const { return m_forceStrength; }
        void SetForceStrength(float value) { m_forceStrength = value; }

        [[nodiscard]] float DampingPerSecond() const { return m_dampingPerSecond; }
        void SetDampingPerSecond(float value) { m_dampingPerSecond = value; }
    private:
        void UpdateViewMatrix();

        DirectX::XMFLOAT3 m_position{ 0.0, 0.0f, -7.5f };
        DirectX::XMFLOAT3 m_right { 1.0f, 0.0f, 0.0f };
        DirectX::XMFLOAT3 m_up { 0.0f, 1.0f, 0.0f };
        DirectX::XMFLOAT3 m_look { 0.0f, 0.0f, 1.0f };

        float m_yaw = 0.0f;
        float m_pitch = 0.0f;

        DirectX::XMFLOAT3 m_velocity { 0.0f, 0.0f, 0.0f };
        DirectX::XMFLOAT3 m_pendingForce { 0.0f, 0.0f, 0.0f };

        float m_mass = 1.0f;
        float m_forceStrength = 30.0f;
        float m_dampingPerSecond = 0.05f;

        float m_fovY = 0.0f;
        float m_aspect = 1.0f;
        float m_nearZ = 0.1f;
        float m_farZ = 100.0f;

        DirectX::XMFLOAT4X4 m_view{};
        DirectX::XMFLOAT4X4 m_proj{};
    };
}