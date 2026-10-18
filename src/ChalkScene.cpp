#include "ChalkScene.h"

#include <utility>

ChalkSceneObject& ChalkScene::Add(const std::wstring& name, ChalkMesh* mesh, ChalkMaterial* material) {
    ChalkSceneObject object;
    object.name = name;
    object.mesh = mesh;
    object.material = material;
    objects_.push_back(std::move(object));
    return objects_.back();
}

bool ChalkScene::Remove(std::size_t index) {
    if (index >= objects_.size()) {
        return false;
    }
    objects_.erase(objects_.begin() + static_cast<std::ptrdiff_t>(index));
    return true;
}

bool ChalkScene::Remove(const std::wstring& name) {
    for (std::size_t i = 0; i < objects_.size(); ++i) {
        if (objects_[i].name == name) {
            objects_.erase(objects_.begin() + static_cast<std::ptrdiff_t>(i));
            return true;
        }
    }
    return false;
}

void ChalkScene::Clear() {
    objects_.clear();
}

ChalkSceneObject* ChalkScene::Get(const std::wstring& name) {
    for (ChalkSceneObject& object : objects_) {
        if (object.name == name) {
            return &object;
        }
    }
    return nullptr;
}

const ChalkSceneObject* ChalkScene::Get(const std::wstring& name) const {
    for (const ChalkSceneObject& object : objects_) {
        if (object.name == name) {
            return &object;
        }
    }
    return nullptr;
}

ChalkSceneObject* ChalkScene::Get(std::size_t index) {
    if (index >= objects_.size()) {
        return nullptr;
    }
    return &objects_[index];
}

const ChalkSceneObject* ChalkScene::Get(std::size_t index) const {
    if (index >= objects_.size()) {
        return nullptr;
    }
    return &objects_[index];
}

ChalkDirectionalLightConstants ChalkScene::BuildLightConstants() const noexcept {
    ChalkDirectionalLightConstants constants;
    constants.direction = light_.direction;
    constants.color = light_.color;
    constants.intensity = light_.intensity;
    return constants;
}
