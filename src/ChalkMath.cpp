#include "ChalkMath.h"

namespace ChalkMath {

XMMATRIX Multiply(const XMMATRIX& left, const XMMATRIX& right) noexcept {
    return DirectX::XMMatrixMultiply(left, right);
}

XMMATRIX Transpose(const XMMATRIX& matrix) noexcept {
    return DirectX::XMMatrixTranspose(matrix);
}

XMMATRIX Inverse(const XMMATRIX& matrix) noexcept {
    return DirectX::XMMatrixInverse(nullptr, matrix);
}

XMMATRIX Translation(float x, float y, float z) noexcept {
    return DirectX::XMMatrixTranslation(x, y, z);
}

XMMATRIX Translation(const XMFLOAT3& offset) noexcept {
    return DirectX::XMMatrixTranslation(offset.x, offset.y, offset.z);
}

XMMATRIX Scaling(float uniformScale) noexcept {
    return DirectX::XMMatrixScaling(uniformScale, uniformScale, uniformScale);
}

XMMATRIX Scaling(float x, float y, float z) noexcept {
    return DirectX::XMMatrixScaling(x, y, z);
}

XMMATRIX RotationX(float radians) noexcept {
    return DirectX::XMMatrixRotationX(radians);
}

XMMATRIX RotationY(float radians) noexcept {
    return DirectX::XMMatrixRotationY(radians);
}

XMMATRIX RotationZ(float radians) noexcept {
    return DirectX::XMMatrixRotationZ(radians);
}

XMMATRIX RotationYawPitchRoll(float yawRadians, float pitchRadians, float rollRadians) noexcept {
    return DirectX::XMMatrixRotationRollPitchYaw(pitchRadians, yawRadians, rollRadians);
}

XMMATRIX LookAtRH(const XMFLOAT3& eye, const XMFLOAT3& target, const XMFLOAT3& up) noexcept {
    return DirectX::XMMatrixLookAtRH(
        DirectX::XMLoadFloat3(&eye),
        DirectX::XMLoadFloat3(&target),
        DirectX::XMLoadFloat3(&up));
}

XMMATRIX PerspectiveFovRH(float fovYRadians, float aspectRatio, float nearZ, float farZ) noexcept {
    return DirectX::XMMatrixPerspectiveFovRH(fovYRadians, aspectRatio, nearZ, farZ);
}

float Length(const XMFLOAT3& vector) noexcept {
    return DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMLoadFloat3(&vector)));
}

float Dot(const XMFLOAT3& left, const XMFLOAT3& right) noexcept {
    return DirectX::XMVectorGetX(
        DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&left), DirectX::XMLoadFloat3(&right)));
}

XMFLOAT3 Cross(const XMFLOAT3& left, const XMFLOAT3& right) noexcept {
    XMFLOAT3 result;
    DirectX::XMStoreFloat3(
        &result,
        DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&left), DirectX::XMLoadFloat3(&right)));
    return result;
}

XMFLOAT3 Normalize(const XMFLOAT3& vector) noexcept {
    XMFLOAT3 result;
    DirectX::XMStoreFloat3(&result, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&vector)));
    return result;
}

XMFLOAT4X4 ToUploadMatrix(const XMMATRIX& matrix) noexcept {
    XMFLOAT4X4 result;
    DirectX::XMStoreFloat4x4(&result, DirectX::XMMatrixTranspose(matrix));
    return result;
}

} // namespace ChalkMath
