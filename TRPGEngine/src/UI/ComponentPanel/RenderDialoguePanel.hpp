#pragma once

#include <imgui.h>
#include <cstring>
#include <string>   // cache labels
#include <vector>   // cache vector
#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/DialogueComponent.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"
#include "Project/ProjectManager.hpp"
#include "Resources/ResourceManager.hpp" // mark unsaved

inline void renderDialogueInspector(const std::shared_ptr<DialogueComponent>& comp) {
    auto& em = EntityManager::get();

    // --- Dialogue Lines ---
    ImGui::Text("Dialogue Lines:");
    for (size_t i = 0; i < comp->lines.size(); ++i) {
        ImGui::PushID(static_cast<int>(i));
        char buffer[256];
        std::strncpy(buffer, comp->lines[i].c_str(), sizeof(buffer));
        buffer[255] = '\0';
        if (ImGui::InputText("Line", buffer, sizeof(buffer))) {
            comp->lines[i] = buffer;
            ResourceManager::get().setUnsavedChanges(true);
        }
        ImGui::PopID();
    }

    if (ImGui::Button("Add Line")) {
        comp->lines.push_back("New line...");
        ResourceManager::get().setUnsavedChanges(true);
    }

    ImGui::Separator();

    // --- Speaker Entity ---
    ImGui::Text("Speaker Entity:");
    if (comp->speaker != INVALID_ENTITY) {
        ImGui::Text("Speaker ID: %u", comp->speaker);
        if (ImGui::Button("Remove Speaker")) {
            comp->speaker = INVALID_ENTITY;
            ResourceManager::get().setUnsavedChanges(true);
        }
    } else {
        ImGui::TextDisabled("None (Drop an entity here)");
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_FILE")) {
            Entity dropped = *(Entity*)payload->Data;
            comp->speaker = dropped;
            ResourceManager::get().setUnsavedChanges(true);
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::Separator();

    // --- Target Flow Node (by name) ---
    ImGui::Text("Target Flow Node:");

    // Combo listing existing scenes
    {
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
            int current = 0;
            if (!comp->targetFlowNode.empty() && comp->targetFlowNode.rfind("@Event:", 0) != 0) {
                for (int i = 1; i < (int)cache.size(); ++i) {
                    if (cache[i] == comp->targetFlowNode) { current = i; break; }
                }
            }
            if (ImGui::Combo("Target Scene", &current, items.data(), (int)items.size())) {
                comp->targetFlowNode = (current == 0) ? std::string() : cache[current];
                ResourceManager::get().setUnsavedChanges(true);
            }
        }
    }

    // Target event in current scene
    {
        Entity node = EditorUI::get() ? EditorUI::get()->getSelectedEntity() : INVALID_ENTITY;
        auto flow = em.getComponent<FlowNodeComponent>(node);
        if (flow) {
            std::vector<std::string> labels;
            std::vector<Entity> ids;
            labels.emplace_back("<None>"); ids.push_back(INVALID_ENTITY);
            for (Entity e : flow->eventSequence) {
                if (e == INVALID_ENTITY) continue;
                std::string tag = "@Event:" + std::to_string((unsigned)e);
                labels.push_back(tag);
                ids.push_back(e);
            }
            std::vector<const char*> items;
            for (auto& s : labels) items.push_back(s.c_str());

            int cur = 0;
            if (!comp->targetFlowNode.empty() && comp->targetFlowNode.rfind("@Event:", 0) == 0) {
                for (size_t i = 1; i < labels.size(); ++i) if (labels[i] == comp->targetFlowNode) { cur = (int)i; break; }
            }
            if (ImGui::Combo("Target Event", &cur, items.data(), (int)items.size())) {
                comp->targetFlowNode = (cur == 0) ? std::string() : labels[cur];
                ResourceManager::get().setUnsavedChanges(true);
            }
        } else {
            ImGui::TextDisabled("Select a FlowNode to pick target event.");
        }
    }

    if (!comp->targetFlowNode.empty()) {
        ImGui::Text("Next: %s", comp->targetFlowNode.c_str());
        if (ImGui::Button("Clear Target")) { comp->targetFlowNode.clear(); ResourceManager::get().setUnsavedChanges(true); }
    } else {
        ImGui::TextDisabled("None (Drop a node here)");
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_FILE")) {
            Entity dropped = *(Entity*)payload->Data;
            auto node = em.getComponent(dropped, ComponentType::FlowNode);
            if (node) {
                comp->targetFlowNode = std::static_pointer_cast<FlowNodeComponent>(node)->name;
                ResourceManager::get().setUnsavedChanges(true);
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (ImGui::Checkbox("Advance On Click", &comp->advanceOnClick)) {
        ResourceManager::get().setUnsavedChanges(true);
    }
}