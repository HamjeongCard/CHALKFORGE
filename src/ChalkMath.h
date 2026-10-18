#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <DirectXMath.h>

namespace ChalkMath {

using DirectX::XMFLOAT2;
using DirectX::XMFLOAT3;
using DirectX::XMFLOAT4;
using DirectX::XMFLOAT4X4;
using DirectX::XMMATRIX;
using DirectX::XMVECTOR;

constexpr float kPi = 3.14159265358979323846f;

inline float DegreesToRadians(float degrees) noexcept {
    return degrees * (kPi / 180.0f);
}

inline float RadiansToDegrees(float radians) noexcept {
    return radians * (180.0f / kPi);
}

inline XMMATRIX Identity() noexcept {
    return DirectX::XMMatrixIdentity();
}

XMMATRIX Multiply(const XMMATRIX& left, const XMMATRIX& right) noexcept;
XMMATRIX Transpose(const XMMATRIX& matrix) noexcept;
XMMATRIX Inverse(const XMMATRIX& matrix) noexcept;

XMMATRIX Translation(float x, float y, float z) noexcept;
XMMATRIX Translation(const XMFLOAT3& offset) noexcept;
XMMATRIX Scaling(float uniformScale) noexcept;
XMMATRIX Scaling(float x, float y, float z) noexcept;
XMMATRIX RotationX(float radians) noexcept;
XMMATRIX RotationY(float radians) noexcept;
XMMATRIX RotationZ(float radians) noexcept;
XMMATRIX RotationYawPitchRoll(float yawRadians, float pitchRadians, float rollRadians) noexcept;

XMMATRIX LookAtRH(const XMFLOAT3& eye, const XMFLOAT3& target, const XMFLOAT3& up) noexcept;
XMMATRIX PerspectiveFovRH(float fovYRadians, float aspectRatio, float nearZ, float farZ) noexcept;

float Length(const XMFLOAT3& vector) noexcept;
float Dot(const XMFLOAT3& left, const XMFLOAT3& right) noexcept;
XMFLOAT3 Cross(const XMFLOAT3& left, const XMFLOAT3& right) noexcept;
XMFLOAT3 Normalize(const XMFLOAT3& vector) noexcept;

XMFLOAT4X4 ToUploadMatrix(const XMMATRIX& matrix) noexcept;

} // namespace ChalkMath
