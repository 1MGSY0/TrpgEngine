#include "BackgroundPanel.h"
#include "Engine/Entity/Components/BackgroundComponent.h"
#include "Engine/Resources/ResourceManager.h"
#include "UI/ImGuiUtils/ImGuiUtils.h"
#include "UI/EditorUI.h"
#include <misc/cpp/imgui_stdlib.h> 

#include <imgui.h>
#include <fstream>
#include <memory>
#include <cstring>
#include <filesystem>
#include <Engine\Entity\Components\BackgroundComponent.h>

void renderBackgroundInspector(std::shared_ptr<BackgroundComponent> background) {
    if (!background) {
        ImGui::Text("No background image selected.");
        return;
    }

    // Editable name
    ImGui::InputText("Name", &background->name);

    // Editable texture path
    ImGui::InputText("Texture Path", &background->texturePath);

    // Slider for scale
    ImGui::DragFloat("Scale", &background->scale, 0.01f, 0.1f, 10.0f);

    // Save button
    if (ImGui::Button("Save")) {
        std::string filePath = background->name + ".jpg";
        std::ofstream file(filePath);
        if (file.is_open()) {
            file << background->toJson().dump(4);
            file.close();
            ImGui::Text("Saved to %s", filePath.c_str());
        } else {
            ImGui::Text("Failed to save to %s", filePath.c_str());
        }
    }
}

// choose filter for background (noise, blur, b&w, etc)
// function to be implemented