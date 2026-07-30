#include "Camera.hpp"

#include <algorithm>
#include <cmath>

using namespace DirectX;

namespace
{
    constexpr float kMaxPitch = XM_PIDIV2 - 0.01f;
}

namespace RenderEngine::Scene
{
    Camera::Camera()
    {
        UpdateViewMatrix();
    }

    void Camera::SetLens(float fovY, float aspect, float nearZ, float farZ)
    {
        m_fovY = fovY;
        m_aspect = aspect;
        m_nearZ = nearZ;
        m_farZ = farZ;
        DirectX::XMStoreFloat4x4(&m_proj, XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ));
    }

    void Camera::SetPosition(float x, float y, float z)
    {
        m_position = XMFLOAT3(x, y, z);
        UpdateViewMatrix();
    }

    void Camera::ApplyMoveForce(const XMFLOAT3& direction, float speedMultiplier)
    {
        if (direction.x == 0.0f && direction.y ==  0.0f && direction.z == 0.0f)
        {
            m_pendingForce = XMFLOAT3(0.0f, 0.0f, 0.0f);
            return;
        }

        const XMVECTOR right = DirectX::XMLoadFloat3(&m_right);
        const XMVECTOR look = DirectX::XMLoadFloat3(&m_look);
        const XMVECTOR worldUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        XMVECTOR combined = DirectX::XMVectorScale(right, direction.x);
        combined = combined + DirectX::XMVectorScale(look, direction.z);
        combined = combined + DirectX::XMVectorScale(worldUp, direction.y);
        combined = DirectX::XMVector3Normalize(combined);

        const XMVECTOR force = DirectX::XMVectorScale(combined, m_forceStrength * speedMultiplier);

        DirectX::XMStoreFloat3(&m_pendingForce, force);
    }

    void Camera::Pitch(float angleRadians)
    {
        m_pitch = std::clamp(m_pitch + angleRadians, -kMaxPitch, kMaxPitch);
    }

    void Camera::RotateY(float angleRadians)
    {
        m_yaw += angleRadians;
    }

    void Camera::Update(float dt)
    {
        if (dt <= 0.0f)
        {
            UpdateViewMatrix();
            return;
        }

        XMVECTOR velocity = DirectX::XMLoadFloat3(&m_velocity);
        const XMVECTOR force = DirectX::XMLoadFloat3(&m_pendingForce);

        const XMVECTOR acceleration = DirectX::XMVectorScale(force, 1.0f / m_mass);
        velocity = velocity + DirectX::XMVectorScale(acceleration, dt);

        const float dampingFactor = std::pow(m_dampingPerSecond, dt);
        velocity = DirectX::XMVectorScale(velocity, dampingFactor);

        XMVECTOR position = DirectX::XMLoadFloat3(&m_position);
        position = position + DirectX::XMVectorScale(velocity, dt);

        DirectX::XMStoreFloat3(&m_velocity, velocity);
        DirectX::XMStoreFloat3(&m_position, position);

        m_pendingForce = XMFLOAT3(0.0f, 0.0f, 0.0f);

        UpdateViewMatrix();
    }

    void Camera::UpdateViewMatrix()
    {
        const XMVECTOR look = DirectX::XMVectorSet(
            std::cos(m_pitch) * std::sin(m_yaw),
            std::sin(m_pitch),
            std::cos(m_pitch) * std::cos(m_yaw),
            0.0f
        );
        const XMVECTOR worldUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        const XMVECTOR right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(worldUp, look));
        const XMVECTOR up = DirectX::XMVector3Cross(look, right);

        DirectX::XMStoreFloat3(&m_look, look);
        DirectX::XMStoreFloat3(&m_right, right);
        DirectX::XMStoreFloat3(&m_up, up);

        const XMVECTOR position = DirectX::XMLoadFloat3(&m_position);
        const XMMATRIX view = DirectX::XMMatrixLookAtLH(position, position + look, up);
        DirectX::XMStoreFloat4x4(&m_view, view);
    }

    XMMATRIX Camera::GetView() const
    {
        return DirectX::XMLoadFloat4x4(&m_view);
    }

    XMMATRIX Camera::GetProj() const
    {
        return DirectX::XMLoadFloat4x4(&m_proj);
    }
}