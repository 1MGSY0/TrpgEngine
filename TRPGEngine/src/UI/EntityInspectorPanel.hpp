#pragma once

#include <memory>
#include <imgui.h>
#include <fstream>
#include <cstring>
#include <filesystem>

#include "UI/EditorUI.hpp"
#include "Engine/RenderSystem/SceneManager.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Entity.hpp"

/**
 * Renders the Inspector UI for a given entity.
 * Dynamically displays known components (Character, Script, etc.)
 * and ignores unknown ones unless extended.
 *
 * @param entity The selected entity.
 * @param em Reference to the active EntityManager.
 */

inline void renderEntityInspector(Entity entity) {
    auto& em = EntityManager::get();

    if (entity == INVALID_ENTITY) {
        ImGui::Text("No entity selected.");
        return;
    }

    // 1. Add Component Section
    if (ImGui::CollapsingHeader("Add Component", ImGuiTreeNodeFlags_DefaultOpen)) {
        static int selectedIndex = 0;
        static std::vector<std::string> keys;
        static std::vector<ComponentType> types;

        if (keys.empty()) {
            for (const auto& [type, info] : ComponentTypeRegistry::getAllInfos()) {
                keys.push_back(info.key);
                types.push_back(type);
            }
        }

        std::vector<const char*> cStrs;
        for (const auto& k : keys) cStrs.push_back(k.c_str());

        ImGui::Combo("Component", &selectedIndex, cStrs.data(), static_cast<int>(cStrs.size()));
        if (ImGui::Button("Attach")) {
            auto selectedType = types[selectedIndex];
            const auto& info = ComponentTypeRegistry::getInfo(selectedType);
            if (info && info->loader) {
                auto component = info->loader(nlohmann::json{});
                em.addComponent(entity, component);
            }
        }
    }

    ImGui::Separator();

    // 2. Display Existing Components
    const auto& components = em.getAllComponents(entity);
    if (components.empty()) {
        ImGui::Text("This entity has no components.");
        return;
    }

    for (const auto& comp : components) {
        ComponentType type = comp->getType();
        const auto* info = ComponentTypeRegistry::getInfo(type);
        if (!info) continue;

        ImGui::PushID(info->key.c_str());
        if (ImGui::CollapsingHeader(info->key.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            if (info->inspectorRenderer) {
                info->inspectorRenderer(comp);
            } else {
                ImGui::Text("No inspector defined for %s", info->key.c_str());
            }
        }
        ImGui::PopID();
        ImGui::Separator();
    }
}
