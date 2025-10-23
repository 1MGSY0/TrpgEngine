#pragma once

#include <imgui.h>
#include <cstring>
#include <string>   // names in combos
#include <vector>   // cache/items
#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/DiceRollComponent.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"
#include "Project/ProjectManager.hpp"
#include "Resources/ResourceManager.hpp" // mark unsaved

inline void renderDiceInspector(const std::shared_ptr<DiceRollComponent>& comp) {
    ImGui::Text("Dice Roll Settings");

    // Dice sides (e.g. D6, D20)
    if (ImGui::InputInt("Dice Sides", &comp->sides)) {
        ResourceManager::get().setUnsavedChanges(true);
    }

    // Success threshold
    if (ImGui::InputInt("Success Threshold", &comp->threshold)) {
        ResourceManager::get().setUnsavedChanges(true);
    }

    // Scene target combos
    auto& em = EntityManager::get();
    Entity metaEntity = ProjectManager::getProjectMetaEntity();
    auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
    if (base) {
        auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);
        static std::vector<std::string> cache;
        static std::vector<const char*> items;
        cache.clear(); items.clear();
        cache.emplace_back("<None>");
        for (Entity e : meta->sceneNodes) {
            auto fn = em.getComponent<FlowNodeComponent>(e);
            cache.emplace_back(fn ? fn->name : std::string("[Missing] ") + std::to_string(e));
        }
        for (auto& s : cache) items.push_back(s.c_str());

        int succ = 0, fail = 0;
        if (!comp->onSuccess.empty() && comp->onSuccess.rfind("@Event:", 0) != 0)
            for (int i = 1; i < (int)cache.size(); ++i) if (cache[i] == comp->onSuccess) succ = i;
        if (!comp->onFailure.empty() && comp->onFailure.rfind("@Event:", 0) != 0)
            for (int i = 1; i < (int)cache.size(); ++i) if (cache[i] == comp->onFailure) fail = i;

        if (ImGui::Combo("On Success -> Scene", &succ, items.data(), (int)items.size())) {
            if (succ == 0 && comp->onSuccess.rfind("@Event:", 0) == 0) {
                // keep event tag if set; scene cleared
            } else {
                comp->onSuccess = (succ == 0) ? std::string() : cache[succ];
                ResourceManager::get().setUnsavedChanges(true);
            }
        }
        if (ImGui::Combo("On Failure -> Scene", &fail, items.data(), (int)items.size())) {
            if (fail == 0 && comp->onFailure.rfind("@Event:", 0) == 0) {
                // keep event tag if set; scene cleared
            } else {
                comp->onFailure = (fail == 0) ? std::string() : cache[fail];
                ResourceManager::get().setUnsavedChanges(true);
            }
        }
    }

    // Event target combos in current scene
    {
        auto& em2 = EntityManager::get();
        Entity node = EditorUI::get() ? EditorUI::get()->getSelectedEntity() : INVALID_ENTITY;
        auto flow = em2.getComponent<FlowNodeComponent>(node);
        if (flow) {
            std::vector<std::string> events; events.emplace_back("<None>");
            for (Entity e : flow->eventSequence) {
                if (e == INVALID_ENTITY) continue;
                events.emplace_back("@Event:" + std::to_string((unsigned)e));
            }
            std::vector<const char*> items; for (auto& s : events) items.push_back(s.c_str());
            int succEvt = 0, failEvt = 0;
            if (!comp->onSuccess.empty() && comp->onSuccess.rfind("@Event:", 0) == 0)
                for (int i = 1; i < (int)events.size(); ++i) if (events[i] == comp->onSuccess) succEvt = i;
            if (!comp->onFailure.empty() && comp->onFailure.rfind("@Event:", 0) == 0)
                for (int i = 1; i < (int)events.size(); ++i) if (events[i] == comp->onFailure) failEvt = i;

            if (ImGui::Combo("On Success -> Event", &succEvt, items.data(), (int)items.size())) {
                comp->onSuccess = (succEvt == 0) ? std::string() : events[succEvt];
                ResourceManager::get().setUnsavedChanges(true);
            }
            if (ImGui::Combo("On Failure -> Event", &failEvt, items.data(), (int)items.size())) {
                comp->onFailure = (failEvt == 0) ? std::string() : events[failEvt];
                ResourceManager::get().setUnsavedChanges(true);
            }
        } else {
            ImGui::TextDisabled("Select a FlowNode to pick target event.");
        }
    }

    // Free-text fallback
    char bufferSuccess[256];
    std::strncpy(bufferSuccess, comp->onSuccess.c_str(), sizeof(bufferSuccess));
    bufferSuccess[255] = '\0';
    if (ImGui::InputText("On Success Trigger (name or @Event:id)", bufferSuccess, sizeof(bufferSuccess))) {
        comp->onSuccess = bufferSuccess;
        ResourceManager::get().setUnsavedChanges(true);
    }
    char bufferFailure[256];
    std::strncpy(bufferFailure, comp->onFailure.c_str(), sizeof(bufferFailure));
    bufferFailure[255] = '\0';
    if (ImGui::InputText("On Failure Trigger (name or @Event:id)", bufferFailure, sizeof(bufferFailure))) {
        comp->onFailure = bufferFailure;
        ResourceManager::get().setUnsavedChanges(true);
    }

    ImGui::Separator();
    ImGui::TextWrapped("Targets can be a Scene name or an event tag like @Event:123 to chain events within this scene.");
}