#pragma once

#include <imgui.h>
#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/Components/TransformComponent.hpp"
#include "Resources/ResourceManager.hpp" // + mark unsaved

inline void renderTransform3DInspector(const std::shared_ptr<TransformComponent>& comp) {
    ImGui::Text("Transform 3D");
    if (ImGui::DragFloat3("Position", &comp->position.x, 0.1f)) ResourceManager::get().setUnsavedChanges(true);
    if (ImGui::DragFloat3("Rotation", &comp->rotation.x, 1.0f)) ResourceManager::get().setUnsavedChanges(true);
    if (ImGui::DragFloat3("Scale",    &comp->scale.x, 0.01f, 0.0f, 10.0f)) ResourceManager::get().setUnsavedChanges(true);
}