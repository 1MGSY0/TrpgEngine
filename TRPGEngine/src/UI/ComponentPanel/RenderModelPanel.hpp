#pragma once

#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/Components/ModelComponent.hpp"

inline void renderModelInspector(const std::shared_ptr<ModelComponent>& comp) {
    ImGui::Text("Model (.obj)");
    ImGui::Checkbox("Is Loaded", &comp->isLoaded);
    ImGui::Checkbox("Is Visible", &comp->isVisible);
}