#include "EntityInspectorPanel.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/CharacterComponent.hpp"
#include "Engine/EntitySystem/Components/ScriptComponent.hpp"

#include "UI/AssetPanels/CharacterPanel.hpp"
#include "UI/AssetPanels/ScriptPanel.hpp"

#include <imgui.h>

void renderEntityInspector(Entity entity, EntityManager& em) {
    if (entity == INVALID_ENTITY) {
        ImGui::Text("No entity selected.");
        return;
    }

    auto components = em.getAllComponents(entity);
    if (components.empty()) {
        ImGui::Text("This entity has no components.");
        return;
    }

    for (auto& comp : components) {
        switch (comp->getType()) {
            case ComponentType::Character:
                renderCharacterInspector(std::static_pointer_cast<CharacterComponent>(comp));
                break;
            case ComponentType::Script:
                renderScriptInspector(std::static_pointer_cast<ScriptComponent>(comp));
                break;
            default:
                ImGui::Text("Unknown or unhandled component type.");
                break;
        }
        ImGui::Separator();
    }
}
