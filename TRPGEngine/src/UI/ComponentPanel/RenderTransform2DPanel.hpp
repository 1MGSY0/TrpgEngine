#pragma once

#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/Components/Transform2DComponent.hpp"

inline void renderTransform2DInspector(const std::shared_ptr<Transform2DComponent>& comp) {
    ImGui::Text("Transform 2D");
    ImGui::DragFloat2("Position", &comp->position.x, 1.0f);
    ImGui::DragFloat2("Size", &comp->size.x, 1.0f);
    ImGui::DragFloat2("Scale", &comp->scale.x, 0.01f, 0.0f, 10.0f);
    ImGui::DragFloat("Rotation", &comp->rotation, 1.0f, -360.0f, 360.0f);
}