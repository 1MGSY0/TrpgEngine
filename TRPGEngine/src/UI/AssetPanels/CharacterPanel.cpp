
#include <imgui.h>
#include <fstream>
#include <memory>
#include <cstring>
#include <filesystem>
#include <misc/cpp/imgui_stdlib.h> 

#include "CharacterPanel.hpp"
#include "Engine/EntitySystem/Components/CharacterComponent.hpp"

#include "UI/ImGuiUtils/ImGuiUtils.h"
#include "UI/EditorUI.h"

void renderCharacterInspector(std::shared_ptr<CharacterComponent> character) {
    if (!character) {
        ImGui::Text("No character selected.");
        return;
    }

    // Editable name
    ImGui::InputText("Name", &character->name);

    // Editable icon image path
    ImGui::InputText("Icon Image", &character->iconImage);

    // --- State Images ---
    if (ImGui::CollapsingHeader("State Images")) {
        for (auto& [state, path] : character->stateImages) {
            ImGui::InputText(state.c_str(), &path);
        }

        static std::string newState;
        static std::string newPath;

        ImGui::InputText("State", &newState);
        ImGui::InputText("Path", &newPath);

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
