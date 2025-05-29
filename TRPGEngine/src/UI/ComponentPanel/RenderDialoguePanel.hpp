#pragma once

#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/Components/DialogueComponent.hpp"

inline void renderDialogueInspector(const std::shared_ptr<DialogueComponent>& comp) {
    auto& em = EntityManager::get();
    
    // --- Dialogue Lines ---
    ImGui::Text("Dialogue Lines:");
    for (size_t i = 0; i < comp->lines.size(); ++i) {
        char buffer[256];
        strncpy(buffer, comp->lines[i].c_str(), sizeof(buffer));
        buffer[255] = '\0';
        if (ImGui::InputText(("Line " + std::to_string(i)).c_str(), buffer, sizeof(buffer)))
            comp->lines[i] = buffer;
    }

    if (ImGui::Button("Add Line")) {
        comp->lines.push_back("New line...");
    }

    ImGui::Separator();

    // --- Speaker Entity ---
    ImGui::Text("Speaker Entity:");
    if (comp->speaker != INVALID_ENTITY) {
        ImGui::Text("Speaker ID: %u", comp->speaker);
        if (ImGui::Button("Remove Speaker")) {
            comp->speaker = INVALID_ENTITY;
        }
    } else {
        ImGui::TextDisabled("None (Drop an entity here)");
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_FILE")) {
            Entity dropped = *(Entity*)payload->Data;
            comp->speaker = dropped;
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::Separator();

    // --- Target Flow Node ---
    ImGui::Text("Target Flow Node:");
    if (!comp->targetFlowNode.empty()) {
        ImGui::Text("Next Node: %s", comp->targetFlowNode.c_str());
        if (ImGui::Button("Remove Target Node")) {
            comp->targetFlowNode.clear();
        }
    } else {
        ImGui::TextDisabled("None (Drop a node here)");
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_FILE")) {
            Entity dropped = *(Entity*)payload->Data;
            auto node = em.getComponent(dropped, ComponentType::FlowNode);
            if (node) {
                comp->targetFlowNode = std::static_pointer_cast<FlowNodeComponent>(node)->name;
            }
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::Checkbox("Advance On Click", &comp->advanceOnClick);
}