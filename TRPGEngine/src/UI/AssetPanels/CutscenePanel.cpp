#include "CutscenePanel.h"
#include "Engine/Entity/Components/CutsceneComponent.h"
#include "Engine/Resources/ResourceManager.h"
#include "UI/ImGuiUtils/ImGuiUtils.h"
#include "UI/EditorUI.h"
#include <misc/cpp/imgui_stdlib.h> 

#include <imgui.h>
#include <fstream>
#include <memory>
#include <cstring>
#include <filesystem>

void renderCutsceneInspector(std::shared_ptr<CutsceneComponent> cutscene) {
    if (!cutscene) {
        ImGui::Text("No video selected.");
        return;
    }

    // Editable name
    ImGui::InputText("Name", &cutscene->name);

    // Editable video file path
    ImGui::InputText("Video File", &cutscene->videoPath);

    // Editable volume
    ImGui::SliderFloat("Volume", &cutscene->volume, 0.0f, 1.0f);

    // Save button
    if (ImGui::Button("Save")) {
        std::string filePath = cutscene->name + ".json";
        std::ofstream file(filePath);
        if (file.is_open()) {
            file << cutscene->toJson().dump(4);
            file.close();
            ImGui::Text("Saved to %s", filePath.c_str());
        } else {
            ImGui::Text("Failed to save to %s", filePath.c_str());
        }
    }
}