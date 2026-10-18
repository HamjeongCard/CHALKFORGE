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

} // namespace ChalkMath
