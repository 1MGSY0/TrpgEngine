#pragma once

#include <memory>
#include <imgui.h>
#include <fstream>
#include <cstring>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>  // + clamp/sort/transform

#include "UI/EditorUI.hpp"
#include "Engine/RenderSystem/SceneManager.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Entity.hpp"
#include "Engine/EntitySystem/ComponentRegistry.hpp"
#include "Resources/ResourceManager.hpp"  // mark unsaved on changes

/**
 * Renders the Inspector UI for a given entity.
 * Dynamically displays known components (Character, Script, etc.)
 * and ignores unknown ones unless extended.
 *
 * @param entity The selected entity.
 * @param em Reference to the active EntityManager.
 */

void EditorUI::renderEntityInspector(Entity entity) {
    // Always reflect the latest global selection to avoid stale caller argument
    if (EditorUI* ui = EditorUI::get()) {
        entity = ui->getSelectedEntity();
    }

    auto& em = EntityManager::get();

    if (entity == INVALID_ENTITY) {
        ImGui::Text("No entity selected.");
        return;
    }

    // 1. Add Component Section
    if (ImGui::CollapsingHeader("Add Component", ImGuiTreeNodeFlags_DefaultOpen)) {
        // Build a fresh list each frame to reflect registry changes and keep scale-friendly
        std::vector<std::pair<std::string, ComponentType>> entries;
        entries.reserve(ComponentTypeRegistry::getAllInfos().size());
        for (const auto& [type, info] : ComponentTypeRegistry::getAllInfos()) {
            entries.emplace_back(info.key, type);
        }
        std::sort(entries.begin(), entries.end(), [](auto& a, auto& b){ return a.first < b.first; });

        static char filter[64] = {0};
        ImGui::InputTextWithHint("##compFilter", "Search component...", filter, IM_ARRAYSIZE(filter));

        // Build filtered arrays for combo
        std::vector<const char*> displayItems;
        std::vector<ComponentType> displayTypes;
        displayItems.reserve(entries.size());
        displayTypes.reserve(entries.size());
        for (auto& e : entries) {
            if (filter[0] != '\0') {
                std::string f = e.first;
                std::string q = filter;
                std::transform(f.begin(), f.end(), f.begin(), ::tolower);
                std::transform(q.begin(), q.end(), q.begin(), ::tolower);
                if (f.find(q) == std::string::npos) continue;
            }
            displayItems.push_back(e.first.c_str());
            displayTypes.push_back(e.second);
        }

        static int selectedIndex = 0;
        if (displayItems.empty()) {
            ImGui::TextDisabled("No components match filter.");
        } else {
            selectedIndex = std::clamp(selectedIndex, 0, (int)displayItems.size() - 1);
            ImGui::Combo("Component##AddComp", &selectedIndex, displayItems.data(), (int)displayItems.size());
            if (ImGui::Button("Attach##AddComp")) {
                const ComponentType selectedType = displayTypes[selectedIndex];
                if (const auto* info = ComponentTypeRegistry::getInfo(selectedType)) {
                    if (info->loader) {
                        // Use safe defaults for components whose loaders expect fields
                        nlohmann::json init = nlohmann::json::object();
                        switch (selectedType) {
                            case ComponentType::Dialogue:
                                init["lines"] = nlohmann::json::array();
                                init["speaker"] = -1;
                                init["advanceOnClick"] = true;
                                init["targetFlowNode"] = "";
                                break;
                            case ComponentType::Choice:
                                init["options"] = nlohmann::json::array();
                                break;
                            case ComponentType::DiceRoll:
                                init["sides"] = 6;
                                init["threshold"] = 1;
                                init["onSuccess"] = "";
                                init["onFailure"] = "";
                                break;
                            default:
                                break;
                        }
                        auto comp = info->loader(init);
                        const auto res = EntityManager::get().addComponent(entity, comp);
                        switch (res) {
                            case EntityManager::AddComponentResult::Ok:
                                comp->Init(entity);
                                setStatusMessage("Attached \"" + info->key + "\" to entity " + std::to_string(entity));
                                ResourceManager::get().setUnsavedChanges(true);
                                break;
                            case EntityManager::AddComponentResult::AlreadyExists:
                                setStatusMessage("Entity already has \"" + info->key + "\" — not added.");
                                break;
                            case EntityManager::AddComponentResult::InvalidEntityId:
                                setStatusMessage("Invalid entity id.");
                                break;
                            case EntityManager::AddComponentResult::EntityNotFound:
                                setStatusMessage("Entity not found — did you create it?");
                                break;
                            case EntityManager::AddComponentResult::NullComponent:
                                setStatusMessage("Factory returned null component.");
                                break;
                        }
                    } else {
                        setStatusMessage("No loader registered for \"" + info->key + "\".");
                    }
                }
            }
        }
    }

    ImGui::Separator();

    // 1b. Remove Component Section (only present components)
    if (ImGui::CollapsingHeader("Remove Component", ImGuiTreeNodeFlags_DefaultOpen)) {
        // Build list of components actually on this entity
        std::vector<std::string> presentKeys;
        std::vector<ComponentType> presentTypes;
        for (const auto& comp : em.getAllComponents(entity)) {
            const auto* info = ComponentTypeRegistry::getInfo(comp->getType());
            if (!info) continue;
            presentKeys.push_back(info->key);
            presentTypes.push_back(comp->getType());
        }

        if (presentKeys.empty()) {
            ImGui::TextDisabled("This entity has no components.");
        } else {
            static int removeIndex = 0;
            std::vector<const char*> cStrs;
            cStrs.reserve(presentKeys.size());
            for (const auto& k : presentKeys) cStrs.push_back(k.c_str());

            // Clamp index BEFORE any vector access
            removeIndex = std::clamp(removeIndex, 0, (int)presentTypes.size() - 1);

            ImGui::Combo("Component##RemoveComp", &removeIndex, cStrs.data(), static_cast<int>(cStrs.size()));

            bool protectedComp = (presentTypes[removeIndex] == ComponentType::ProjectMetadata);
            if (protectedComp) {
                ImGui::TextDisabled("(ProjectMetadata cannot be removed)");
            }

            if (ImGui::Button("Remove##RemoveComp") && !protectedComp) {
                const auto typeToRemove = presentTypes[removeIndex];
                const auto* info = ComponentTypeRegistry::getInfo(typeToRemove);
                if (em.removeComponent(entity, typeToRemove)) {
                    setStatusMessage(std::string("Removed component: ") + (info ? info->key : "<unknown>"));
                    ResourceManager::get().setUnsavedChanges(true);
                    // Clamp index after removal
                    removeIndex = std::clamp(removeIndex, 0, (int)presentTypes.size() - 2);
                } else {
                    setStatusMessage("Failed to remove component.");
                }
            }
        }
    }

    ImGui::Separator();

    // 2. Display Existing Components with their inspectors
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
