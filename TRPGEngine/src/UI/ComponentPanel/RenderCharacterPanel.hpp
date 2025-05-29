#pragma once

#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/Components/CharacterComponent.hpp"

inline void renderCharacterInspector(const std::shared_ptr<CharacterComponent>& character) {
    if (!character) {
        ImGui::Text("No character selected.");
        return;
    }

    // Editable name
    ImGui::InputText("Name", &character->name);

    // --- Icon Image with drag-drop ---
    ImGui::InputText("Icon Image", &character->iconImage);
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH")) {
            const char* path = static_cast<const char*>(payload->Data);
            character->iconImage = path;
        }
        ImGui::EndDragDropTarget();
    }

    // --- State Images ---
    if (ImGui::CollapsingHeader("State Images")) {
        for (auto& [state, path] : character->stateImages) {
            ImGui::PushID(state.c_str());

            ImGui::InputText("Path", &path);
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH")) {
                    const char* dropped = static_cast<const char*>(payload->Data);
                    path = dropped;
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::SameLine();
            ImGui::TextDisabled("State: %s", state.c_str());

            ImGui::PopID();
        }

        static std::string newState;
        static std::string newPath;

        ImGui::InputText("State", &newState);
        ImGui::InputText("Path", &newPath);

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH")) {
                const char* dropped = static_cast<const char*>(payload->Data);
                newPath = dropped;
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::Button("Add State")) {
            if (!newState.empty() && !newPath.empty()) {
                character->stateImages[newState] = newPath;
                newState.clear();
                newPath.clear();
            }
        }
    }

    // --- Stats ---
    if (ImGui::CollapsingHeader("Stats")) {
        for (auto& [key, value] : character->stats) {
            int v = value;
            if (ImGui::DragInt(key.c_str(), &v)) {
                character->stats[key] = v;
            }
        }

        static std::string newStatKey;
        static int newStatValue = 0;

        ImGui::InputText("New Stat", &newStatKey);
        ImGui::DragInt("Value", &newStatValue);
        if (ImGui::Button("Add Stat")) {
            if (!newStatKey.empty()) {
                character->stats[newStatKey] = newStatValue;
                newStatKey.clear();
                newStatValue = 0;
            }
        }
    }
}