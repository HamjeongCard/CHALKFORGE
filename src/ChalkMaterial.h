#pragma once

#include "ChalkMath.h"

#include <windows.h>
#include <d3d12.h>
#include <wrl/client.h>

struct ChalkMaterialConstants {
    ChalkMath::XMFLOAT4 baseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    ChalkMath::XMFLOAT4 emissive = { 0.0f, 0.0f, 0.0f, 1.0f };
    float roughness = 0.5f;
    float metallic = 0.0f;
    float specularPower = 1.0f;
    float opacity = 1.0f;
};

class ChalkMaterial {
public:
    ChalkMaterial();
    ~ChalkMaterial();

    ChalkMaterial(const ChalkMaterial&) = delete;
    ChalkMaterial& operator=(const ChalkMaterial&) = delete;

    bool Create(ID3D12Device* device, const wchar_t* name = nullptr);
    void Reset();

    bool IsValid() const noexcept;

    const ChalkMaterialConstants& Constants() const noexcept { return constants_; }

    void SetBaseColor(const ChalkMath::XMFLOAT4& color) noexcept { constants_.baseColor = color; }
    void SetEmissive(const ChalkMath::XMFLOAT4& color) noexcept { constants_.emissive = color; }
    void SetRoughness(float value) noexcept { constants_.roughness = value; }
    void SetMetallic(float value) noexcept { constants_.metallic = value; }
    void SetSpecularPower(float value) noexcept { constants_.specularPower = value; }
    void SetOpacity(float value) noexcept { constants_.opacity = value; }

    bool Upload();
    void Bind(ID3D12GraphicsCommandList* commandList, UINT rootParameterIndex) const;

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer_;
    ChalkMaterialConstants constants_ = { };
};
