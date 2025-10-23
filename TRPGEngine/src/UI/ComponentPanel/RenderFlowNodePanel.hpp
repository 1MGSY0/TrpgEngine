#pragma once
#include <imgui.h>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <json.hpp>
#include <unordered_set>
#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/Entity.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/EntitySystem/Components/CharacterComponent.hpp"
#include "Engine/EntitySystem/Components/BackgroundComponent.hpp"
#include "Engine/EntitySystem/Components/DialogueComponent.hpp"
#include "Engine/EntitySystem/Components/ChoiceComponent.hpp"
#include "Engine/EntitySystem/Components/DiceRollComponent.hpp"
#include "Engine/EntitySystem/Components/UIButtonComponent.hpp"
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"
#include "Project/ProjectManager.hpp"
#include "Resources/ResourceManager.hpp"
#include "Engine/EntitySystem/ComponentRegistry.hpp"

inline void renderFlowNodeInspector(const std::shared_ptr<FlowNodeComponent>& comp) {
    auto& em = EntityManager::get();
    const Entity self = EditorUI::get() ? EditorUI::get()->getSelectedEntity() : INVALID_ENTITY;

    ImGui::Text("Flow Node: %s", comp->name.c_str());
    ImGui::Separator();

    char nameBuffer[128];
    std::strncpy(nameBuffer, comp->name.c_str(), sizeof(nameBuffer));
    nameBuffer[sizeof(nameBuffer) - 1] = '\0';
    if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
        comp->name = nameBuffer;
        ResourceManager::get().setUnsavedChanges(true);
    }

    // Sync start flag with ProjectMeta using the inspected entity (self)
    {
        Entity metaEntity = ProjectManager::getProjectMetaEntity();
        auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
        if (base && self != INVALID_ENTITY) {
            auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);
            bool isStartNow = (meta->startNode == self);
            bool startChecked = isStartNow;
            if (ImGui::Checkbox("Start Node", &startChecked)) {
                if (startChecked) {
                    // Unset previous
                    if (meta->startNode != INVALID_ENTITY && meta->startNode != self) {
                        if (auto prev = em.getComponent<FlowNodeComponent>(meta->startNode))
                            prev->isStart = false;
                    }
                    // Set new
                    meta->startNode = self;
                    comp->isStart = true;
                    ResourceManager::get().setUnsavedChanges(true);
                } else {
                    // If this node was start and unchecked, clear meta flag
                    if (meta->startNode == self) {
                        meta->startNode = INVALID_ENTITY;
                        ResourceManager::get().setUnsavedChanges(true);
                    }
                    comp->isStart = false;
                }
            } else {
                // Keep component flag in sync for rendering consistency
                comp->isStart = isStartNow;
            }
        } else {
            ImGui::Checkbox("Start Node", &comp->isStart);
        }
    }

    ImGui::SameLine();
    if (ImGui::Checkbox("End Node", &comp->isEnd)) {
        ResourceManager::get().setUnsavedChanges(true);
    }

    // Next node picker (links in the flow graph)
    {
        Entity metaEntity = ProjectManager::getProjectMetaEntity();
        auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
        if (base) {
            auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);

            // Build list of candidates (Entity-as-int) and labels
            std::vector<std::pair<int, std::string>> options;
            options.emplace_back(-1, std::string("<None>"));
            for (Entity e : meta->sceneNodes) {
                auto f = em.getComponent<FlowNodeComponent>(e);
                std::string label = f ? f->name : std::string("[Missing] ") + std::to_string((unsigned)e);
                options.emplace_back(static_cast<int>(e), label);
            }

            // Current preview label
            int current = comp->nextNode;
            std::string preview = "<None>";
            for (const auto& p : options) {
                if (p.first == current) { preview = p.second; break; }
            }

            if (ImGui::BeginCombo("Next Node", preview.c_str())) {
                for (const auto& p : options) {
                    bool selected = (p.first == current);
                    if (ImGui::Selectable(p.second.c_str(), selected)) {
                        comp->nextNode = p.first;
                        ResourceManager::get().setUnsavedChanges(true);
                        current = p.first;
                    }
                    if (selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        } else {
            // Fallback: simple int input (rare; ProjectMeta missing)
            ImGui::InputInt("Next Auto Node", reinterpret_cast<int*>(&comp->nextNode));
        }
    }

    // Characters
    ImGui::Separator();
    ImGui::Text("Characters in Scene:");
    for (int i = 0; i < comp->characters.size(); ++i) {
        Entity charId = comp->characters[i];
        ImGui::BulletText("ID %d", charId);
        ImGui::SameLine();
        if (ImGui::SmallButton((std::string("Inspect##char") + std::to_string(charId)).c_str()))
            EditorUI::get()->setSelectedEntity(charId);
        ImGui::SameLine();
        if (ImGui::SmallButton((std::string("Remove##char") + std::to_string(charId)).c_str())) {
            comp->characters.erase(comp->characters.begin() + i);
            i--;
        }
    }
    ImGui::TextDisabled("Drag & drop Character entity here.");
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_FILE")) {
            Entity dropped = *(Entity*)payload->Data;
            if (em.getComponent<CharacterComponent>(dropped))
                comp->characters.push_back(dropped);
        }
        ImGui::EndDragDropTarget();
    }

    // Backgrounds
    ImGui::Separator();
    ImGui::Text("Background(s):");
    for (int i = 0; i < comp->backgroundEntities.size(); ++i) {
        Entity bgId = comp->backgroundEntities[i];
        ImGui::BulletText("ID %d", bgId);
        ImGui::SameLine();
        if (ImGui::SmallButton((std::string("Inspect##bg") + std::to_string(bgId)).c_str()))
            EditorUI::get()->setSelectedEntity(bgId);
        ImGui::SameLine();
        if (ImGui::SmallButton((std::string("Remove##bg") + std::to_string(bgId)).c_str())) {
            comp->backgroundEntities.erase(comp->backgroundEntities.begin() + i);
            i--;
        }
    }
    ImGui::TextDisabled("Drag & drop Background entity here.");
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_FILE")) {
            Entity dropped = *(Entity*)payload->Data;
            if (em.getComponent<BackgroundComponent>(dropped))
                comp->backgroundEntities.push_back(dropped);
        }
        ImGui::EndDragDropTarget();
    }

    // UI Layer
    ImGui::Separator();
    ImGui::Text("UI Layer Entities:");
    for (int i = 0; i < comp->uiLayer.size(); ++i) {
        Entity e = comp->uiLayer[i];
        ImGui::BulletText("UI ID %d", e);
        ImGui::SameLine();
        if (ImGui::SmallButton((std::string("Inspect##ui") + std::to_string(e)).c_str()))
            EditorUI::get()->setSelectedEntity(e);
        ImGui::SameLine();
        if (ImGui::SmallButton((std::string("Remove##ui") + std::to_string(e)).c_str())) {
            comp->uiLayer.erase(comp->uiLayer.begin() + i);
            i--;
        }
    }
    ImGui::TextDisabled("Drop UIButton or Dialogue entity here.");
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_FILE")) {
            Entity dropped = *(Entity*)payload->Data;
            if (em.getComponent<UIButtonComponent>(dropped) || em.getComponent<DialogueComponent>(dropped))
                comp->uiLayer.push_back(dropped);
        }
        ImGui::EndDragDropTarget();
    }

    // Object Layer
    ImGui::Separator();
    ImGui::Text("Object Layer Entities:");
    for (int i = 0; i < comp->objectLayer.size(); ++i) {
        Entity e = comp->objectLayer[i];
        ImGui::BulletText("3D ID %d", e);
        ImGui::SameLine();
        if (ImGui::SmallButton((std::string("Inspect##obj") + std::to_string(e)).c_str()))
            EditorUI::get()->setSelectedEntity(e);
        ImGui::SameLine();
        if (ImGui::SmallButton((std::string("Remove##obj") + std::to_string(e)).c_str())) {
            comp->objectLayer.erase(comp->objectLayer.begin() + i);
            i--;
        }
    }
    ImGui::TextDisabled("Drop 3D Object entities here.");
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_FILE")) {
            Entity dropped = *(Entity*)payload->Data;
            // You can define custom logic to check for 3D object type components
            comp->objectLayer.push_back(dropped);
        }
        ImGui::EndDragDropTarget();
    }

    // Helper: locate owner scene and move events uniquely between scenes
    auto findOwnerScene = [&](Entity evt)->Entity {
        Entity metaEntity = ProjectManager::getProjectMetaEntity();
        auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
        if (!base) return INVALID_ENTITY;
        auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);
        for (Entity nodeId : meta->sceneNodes) {
            auto fn = em.getComponent<FlowNodeComponent>(nodeId);
            if (!fn) continue;
            for (Entity e : fn->eventSequence) if (e == evt) return nodeId;
        }
        return INVALID_ENTITY;
    };
    auto moveEventTo = [&](Entity evt, Entity toNode)->bool {
        if (evt == INVALID_ENTITY || toNode == INVALID_ENTITY) return false;
        Entity owner = findOwnerScene(evt);
        if (owner == toNode) return true;
        if (owner != INVALID_ENTITY) {
            if (auto ownFn = em.getComponent<FlowNodeComponent>(owner)) {
                ownFn->eventSequence.erase(std::remove(ownFn->eventSequence.begin(), ownFn->eventSequence.end(), evt), ownFn->eventSequence.end());
            }
        }
        if (auto dst = em.getComponent<FlowNodeComponent>(toNode)) {
            dst->eventSequence.push_back(evt);
            ResourceManager::get().setUnsavedChanges(true);
            return true;
        }
        return false;
    };

    // REMOVE: Event Flow editor was here. Shifted to Flow -> Events tab.
    ImGui::Separator();
    ImGui::TextDisabled("Manage event order and results in Flow -> Events tab.");

    // Removed the large Event Flow list, quick result editors, and aggregated links summary.
}