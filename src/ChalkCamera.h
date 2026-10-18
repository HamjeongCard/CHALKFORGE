#pragma once

#include "ChalkMath.h"

class ChalkCamera {
public:
    ChalkCamera() = default;

    void SetPosition(const ChalkMath::XMFLOAT3& position) noexcept;
    void SetTarget(const ChalkMath::XMFLOAT3& target) noexcept;
    void SetUp(const ChalkMath::XMFLOAT3& up) noexcept;

    void SetPerspective(float fovYRadians, float aspectRatio, float nearZ, float farZ) noexcept;
    void SetFovY(float fovYRadians) noexcept { fovYRadians_ = fovYRadians; }
    void SetAspectRatio(float aspectRatio) noexcept { aspectRatio_ = aspectRatio; }
    void SetClipPlanes(float nearZ, float farZ) noexcept;

    const ChalkMath::XMFLOAT3& Position() const noexcept { return position_; }
    const ChalkMath::XMFLOAT3& Target() const noexcept { return target_; }
    const ChalkMath::XMFLOAT3& Up() const noexcept { return up_; }

    float FovYRadians() const noexcept { return fovYRadians_; }
    float AspectRatio() const noexcept { return aspectRatio_; }
    float NearZ() const noexcept { return nearZ_; }
    float FarZ() const noexcept { return farZ_; }

    ChalkMath::XMMATRIX GetViewMatrix() const noexcept;
    ChalkMath::XMMATRIX GetProjectionMatrix() const noexcept;
    ChalkMath::XMMATRIX GetViewProjectionMatrix() const noexcept;

    void MoveForward(float distance) noexcept;
    void MoveRight(float distance) noexcept;
    void Orbit(float yawRadians, float pitchRadians) noexcept;

private:
    ChalkMath::XMFLOAT3 position_{ 0.0f, 0.0f, 5.0f };
    ChalkMath::XMFLOAT3 target_{ 0.0f, 0.0f, 0.0f };
    ChalkMath::XMFLOAT3 up_{ 0.0f, 1.0f, 0.0f };

    float fovYRadians_ = ChalkMath::DegreesToRadians(60.0f);
    float aspectRatio_ = 16.0f / 9.0f;
    float nearZ_ = 0.1f;
    float farZ_ = 1000.0f;
};
