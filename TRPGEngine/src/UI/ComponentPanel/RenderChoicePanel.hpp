#pragma once

#include <imgui.h>
#include <cstring>
#include <string>
#include <vector>
#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/ChoiceComponent.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"
#include "Project/ProjectManager.hpp"
#include "Resources/ResourceManager.hpp" // + mark unsaved

inline void renderChoiceInspector(const std::shared_ptr<ChoiceComponent>& comp) {
    ImGui::Text("Choice Options:");

    auto& em = EntityManager::get();

    // Resolve ProjectMeta and current FlowNode before use below
    Entity metaEntity = ProjectManager::getProjectMetaEntity();
    auto metaBase = em.getComponent(metaEntity, ComponentType::ProjectMetadata);

    Entity node = EditorUI::get() ? EditorUI::get()->getSelectedEntity() : INVALID_ENTITY;
    auto flow = em.getComponent<FlowNodeComponent>(node);

    for (size_t i = 0; i < comp->options.size(); ++i) {
        ImGui::PushID(static_cast<int>(i));
        auto& opt = comp->options[i];

        // Split text and parse existing target
        std::string baseText = opt.text;
        std::string targetName; // could be scene or @Event:ID
        const std::string delim = " -> ";
        if (size_t pos = baseText.rfind(delim); pos != std::string::npos) {
            targetName = baseText.substr(pos + delim.size());
            baseText = baseText.substr(0, pos);
        }

        // Text
        char buffer[256];
        std::strncpy(buffer, baseText.c_str(), sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        if (ImGui::InputText("Text", buffer, sizeof(buffer))) {
            baseText = buffer;
            ResourceManager::get().setUnsavedChanges(true);
        }

        // Target Scene
        if (metaBase) {
            auto meta = std::static_pointer_cast<ProjectMetaComponent>(metaBase);
            std::vector<std::string> scenes; scenes.emplace_back("<None>");
            for (Entity e : meta->sceneNodes) {
                auto fn = em.getComponent<FlowNodeComponent>(e);
                scenes.emplace_back(fn ? fn->name : std::string("[Missing] ") + std::to_string(e));
            }
            std::vector<const char*> items; for (auto& s : scenes) items.push_back(s.c_str());
            int curScene = 0;
            if (!targetName.empty() && targetName.rfind("@Event:", 0) != 0) {
                for (int idx = 1; idx < (int)scenes.size(); ++idx) if (scenes[idx] == targetName) { curScene = idx; break; }
            }
            if (ImGui::Combo("Target Scene", &curScene, items.data(), (int)items.size())) {
                targetName = (curScene == 0) ? std::string() : scenes[curScene];
                ResourceManager::get().setUnsavedChanges(true);
            }
        } else {
            ImGui::TextDisabled("No ProjectMeta found.");
        }

        // Target Event (in this FlowNode)
        if (flow) {
            std::vector<std::string> events; std::vector<Entity> ids;
            events.emplace_back("<None>"); ids.push_back(INVALID_ENTITY);
            for (Entity e : flow->eventSequence) {
                if (e == INVALID_ENTITY) continue;
                events.emplace_back("@Event:" + std::to_string((unsigned)e));
                ids.push_back(e);
            }
            std::vector<const char*> items; for (auto& s : events) items.push_back(s.c_str());
            int curEvt = 0;
            if (!targetName.empty() && targetName.rfind("@Event:", 0) == 0) {
                for (int idx = 1; idx < (int)events.size(); ++idx) if (events[idx] == targetName) { curEvt = idx; break; }
            }
            if (ImGui::Combo("Target Event", &curEvt, items.data(), (int)items.size())) {
                targetName = (curEvt == 0) ? targetName : events[curEvt];
                if (curEvt == 0 && targetName.rfind("@Event:", 0) == 0) targetName.clear();
                ResourceManager::get().setUnsavedChanges(true);
            }
        } else {
            ImGui::TextDisabled("Select a FlowNode to pick target event.");
        }

        // Commit final text
        opt.text = baseText;
        if (!targetName.empty()) opt.text += " -> " + targetName;

        ImGui::Text("Trigger enum: %d", static_cast<int>(opt.trigger));

        // Remove Option
        if (ImGui::SmallButton("Remove Option")) {
            comp->options.erase(comp->options.begin() + i);
            ResourceManager::get().setUnsavedChanges(true);
            ImGui::PopID();
            --i; continue;
        }
        ImGui::Separator();
        ImGui::PopID();
    }

    if (ImGui::Button("Add Option")) {
        comp->options.push_back({"New choice", ComponentType::Unknown});
        ResourceManager::get().setUnsavedChanges(true);
    }
}