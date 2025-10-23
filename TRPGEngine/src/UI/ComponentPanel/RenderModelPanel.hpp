#pragma once

#include <imgui.h>
#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/Components/ModelComponent.hpp"
#include "Resources/ResourceManager.hpp" // + mark unsaved

inline void renderModelInspector(const std::shared_ptr<ModelComponent>& comp) {
    ImGui::Text("Model (.obj)");
    if (ImGui::Checkbox("Is Loaded", &comp->isLoaded)) {
        ResourceManager::get().setUnsavedChanges(true);
    }
    if (ImGui::Checkbox("Is Visible", &comp->isVisible)) {
        ResourceManager::get().setUnsavedChanges(true);
    }
}