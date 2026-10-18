#include "ChalkCamera.h"

namespace {

DirectX::XMVECTOR ForwardDirection(const ChalkMath::XMFLOAT3& position,
                                   const ChalkMath::XMFLOAT3& target) noexcept {
    DirectX::XMVECTOR direction =
        DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&target), DirectX::XMLoadFloat3(&position));
    return DirectX::XMVector3Normalize(direction);
}

} // namespace

void ChalkCamera::SetPosition(const ChalkMath::XMFLOAT3& position) noexcept {
    position_ = position;
}

void ChalkCamera::SetTarget(const ChalkMath::XMFLOAT3& target) noexcept {
    target_ = target;
}

void ChalkCamera::SetUp(const ChalkMath::XMFLOAT3& up) noexcept {
    up_ = up;
}

void ChalkCamera::SetPerspective(float fovYRadians, float aspectRatio, float nearZ, float farZ) noexcept {
    fovYRadians_ = fovYRadians;
    aspectRatio_ = aspectRatio;
    nearZ_ = nearZ;
    farZ_ = farZ;
}

void ChalkCamera::SetClipPlanes(float nearZ, float farZ) noexcept {
    nearZ_ = nearZ;
    farZ_ = farZ;
}

ChalkMath::XMMATRIX ChalkCamera::GetViewMatrix() const noexcept {
    return ChalkMath::LookAtRH(position_, target_, up_);
}

ChalkMath::XMMATRIX ChalkCamera::GetProjectionMatrix() const noexcept {
    return ChalkMath::PerspectiveFovRH(fovYRadians_, aspectRatio_, nearZ_, farZ_);
}

ChalkMath::XMMATRIX ChalkCamera::GetViewProjectionMatrix() const noexcept {
    return ChalkMath::Multiply(GetViewMatrix(), GetProjectionMatrix());
}

void ChalkCamera::MoveForward(float distance) noexcept {
    DirectX::XMVECTOR offset = DirectX::XMVectorScale(ForwardDirection(position_, target_), distance);
    DirectX::XMStoreFloat3(&position_, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position_), offset));
    DirectX::XMStoreFloat3(&target_, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&target_), offset));
}

void ChalkCamera::MoveRight(float distance) noexcept {
    DirectX::XMVECTOR right = DirectX::XMVector3Normalize(
        DirectX::XMVector3Cross(ForwardDirection(position_, target_), DirectX::XMLoadFloat3(&up_)));
    DirectX::XMVECTOR offset = DirectX::XMVectorScale(right, distance);
    DirectX::XMStoreFloat3(&position_, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&position_), offset));
    DirectX::XMStoreFloat3(&target_, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&target_), offset));
}

void ChalkCamera::Orbit(float yawRadians, float pitchRadians) noexcept {
    DirectX::XMVECTOR target = DirectX::XMLoadFloat3(&target_);
    DirectX::XMVECTOR offset = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&position_), target);

    DirectX::XMVECTOR worldUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    offset = DirectX::XMVector3TransformNormal(offset, DirectX::XMMatrixRotationAxis(worldUp, yawRadians));

    DirectX::XMVECTOR right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(worldUp, offset));
    offset = DirectX::XMVector3TransformNormal(offset, DirectX::XMMatrixRotationAxis(right, pitchRadians));

    DirectX::XMStoreFloat3(&position_, DirectX::XMVectorAdd(target, offset));
}
