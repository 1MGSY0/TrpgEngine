#include "ObjectPanel.h"
#include "Engine/Entity/Components/ObjectComponent.h"
#include "Engine/Resources/ResourceManager.h"
#include "UI/ImGuiUtils/ImGuiUtils.h"
#include "UI/EditorUI.h"
#include <misc/cpp/imgui_stdlib.h> 

#include <imgui.h>
#include <fstream>
#include <memory>
#include <cstring>
#include <filesystem>
#include <Engine\Entity\Components\ObjectComponent.h>

void renderObjectInspector(std::shared_ptr<ObjectComponent> obj) {
    if (!obj) {
        ImGui::Text("No object/item selected.");
        return;
    }

    // Editable name
    ImGui::InputText("Name", &obj->name);

    // Editable state images
    if (ImGui::CollapsingHeader("State Images")) {
        for (auto& [state, path] : obj->stateImages) {
            ImGui::InputText(state.c_str(), &path);
        }

        static std::string newState;
        static std::string newPath;

        ImGui::InputText("State", &newState);
        ImGui::InputText("Path", &newPath);

        if (ImGui::Button("Add State")) {
            if (!newState.empty() && !newPath.empty()) {
                obj->stateImages[newState] = newPath;
                newState.clear();
                newPath.clear();
            }
        }
    }

    // Save button
    if (ImGui::Button("Save")) {
        std::string filePath = obj->name + ".jpg";
        std::ofstream file(filePath);
        if (file.is_open()) {
            file << obj->toJson().dump(4);
            file.close();
            ImGui::Text("Saved to %s", filePath.c_str());
        } else {
            ImGui::Text("Failed to save to %s", filePath.c_str());
        }
    }
}