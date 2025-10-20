#pragma once

#include <imgui.h>
#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/Components/TransformComponent.hpp"

inline void renderTransform3DInspector(const std::shared_ptr<TransformComponent>& comp) {
    ImGui::Text("Transform 3D");
    ImGui::DragFloat3("Position", &comp->position.x, 0.1f);
    ImGui::DragFloat3("Rotation", &comp->rotation.x, 1.0f);
    ImGui::DragFloat3("Scale",    &comp->scale.x, 0.01f, 0.0f, 10.0f);
}