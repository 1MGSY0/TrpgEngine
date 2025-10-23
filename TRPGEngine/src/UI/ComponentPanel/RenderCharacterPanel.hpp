#pragma once

#include <imgui.h>
#include <cstring>
#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/Components/CharacterComponent.hpp"
#include "Resources/ResourceManager.hpp" // + mark unsaved

inline void renderCharacterInspector(const std::shared_ptr<CharacterComponent>& character) {
    if (!character) {
        ImGui::Text("No character selected.");
        return;
    }

    // Editable name (buffered)
    {
        char nameBuf[128];
        std::strncpy(nameBuf, character->name.c_str(), sizeof(nameBuf));
        nameBuf[sizeof(nameBuf) - 1] = '\0';
        if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
            character->name = nameBuf;
            ResourceManager::get().setUnsavedChanges(true);
        }
    }

    // Icon Image with drag-drop (buffered)
    {
        char iconBuf[260];
        std::strncpy(iconBuf, character->iconImage.c_str(), sizeof(iconBuf));
        iconBuf[sizeof(iconBuf) - 1] = '\0';
        if (ImGui::InputText("Icon Image", iconBuf, sizeof(iconBuf))) {
            character->iconImage = iconBuf;
            ResourceManager::get().setUnsavedChanges(true);
        }
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH")) {
                const char* path = static_cast<const char*>(payload->Data);
                character->iconImage = path;
                ResourceManager::get().setUnsavedChanges(true);
            }
            ImGui::EndDragDropTarget();
        }
    }

    // State Images
    if (ImGui::CollapsingHeader("State Images")) {
        for (auto& kv : character->stateImages) {
            const std::string& state = kv.first;
            std::string& path = kv.second;

            ImGui::PushID(state.c_str());

            char pathBuf[260];
            std::strncpy(pathBuf, path.c_str(), sizeof(pathBuf));
            pathBuf[sizeof(pathBuf) - 1] = '\0';
            if (ImGui::InputText("Path", pathBuf, sizeof(pathBuf))) {
                path = pathBuf;
                ResourceManager::get().setUnsavedChanges(true);
            }
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH")) {
                    const char* dropped = static_cast<const char*>(payload->Data);
                    path = dropped;
                    ResourceManager::get().setUnsavedChanges(true);
                }
                ImGui::EndDragDropTarget();
            }
            ImGui::SameLine();
            ImGui::TextDisabled("State: %s", state.c_str());

            ImGui::PopID();
        }

        static char newState[64] = {0};
        static char newPath[260] = {0};

        ImGui::InputText("State", newState, sizeof(newState));
        ImGui::InputText("Path", newPath, sizeof(newPath));

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH")) {
                const char* dropped = static_cast<const char*>(payload->Data);
                std::strncpy(newPath, dropped, sizeof(newPath));
                newPath[sizeof(newPath) - 1] = '\0';
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::Button("Add State")) {
            if (newState[0] != '\0' && newPath[0] != '\0') {
                character->stateImages[newState] = newPath;
                newState[0] = '\0';
                newPath[0] = '\0';
                ResourceManager::get().setUnsavedChanges(true);
            }
        }
    }

    // Stats
    if (ImGui::CollapsingHeader("Stats")) {
        for (auto& kv : character->stats) {
            const std::string& key = kv.first;
            int& value = kv.second;
            if (ImGui::DragInt(key.c_str(), &value)) {
                ResourceManager::get().setUnsavedChanges(true);
            }
        }

        static char newStatKey[64] = {0};
        static int newStatValue = 0;

        ImGui::InputText("New Stat", newStatKey, sizeof(newStatKey));
        ImGui::DragInt("Value", &newStatValue);
        if (ImGui::Button("Add Stat")) {
            if (newStatKey[0] != '\0') {
                character->stats[newStatKey] = newStatValue;
                newStatKey[0] = '\0';
                newStatValue = 0;
                ResourceManager::get().setUnsavedChanges(true);
            }
        }
    }
}