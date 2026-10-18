#pragma once

#include "ChalkMath.h"

#include <cstddef>
#include <string>
#include <vector>

class ChalkMesh;
class ChalkMaterial;

struct ChalkDirectionalLight {
    ChalkMath::XMFLOAT4 direction = { 0.0f, -1.0f, 0.0f, 0.0f };
    ChalkMath::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    float intensity = 1.0f;
};

struct ChalkDirectionalLightConstants {
    ChalkMath::XMFLOAT4 direction = { 0.0f, -1.0f, 0.0f, 0.0f };
    ChalkMath::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    float intensity = 1.0f;
    float padding0 = 0.0f;
    float padding1 = 0.0f;
    float padding2 = 0.0f;
};

struct ChalkSceneObject {
    std::wstring name;
    ChalkMath::XMFLOAT4X4 world = { 1.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 1.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 1.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 1.0f };
    ChalkMesh* mesh = nullptr;
    ChalkMaterial* material = nullptr;
};

class ChalkScene {
public:
    ChalkSceneObject& Add(const std::wstring& name, ChalkMesh* mesh, ChalkMaterial* material);
    bool Remove(std::size_t index);
    bool Remove(const std::wstring& name);
    void Clear();

    ChalkSceneObject* Get(const std::wstring& name);
    const ChalkSceneObject* Get(const std::wstring& name) const;
    ChalkSceneObject* Get(std::size_t index);
    const ChalkSceneObject* Get(std::size_t index) const;

    std::size_t Count() const noexcept { return objects_.size(); }
    const std::vector<ChalkSceneObject>& Objects() const noexcept { return objects_; }

    ChalkDirectionalLight& Light() noexcept { return light_; }
    const ChalkDirectionalLight& Light() const noexcept { return light_; }

    ChalkDirectionalLightConstants BuildLightConstants() const noexcept;

private:
    std::vector<ChalkSceneObject> objects_;
    ChalkDirectionalLight light_;
};
