#include "AudioPanel.h"
#include "Engine/Entity/Components/AudioComponent.h"
#include "Engine/Resources/ResourceManager.h"
#include "UI/ImGuiUtils/ImGuiUtils.h"
#include "UI/EditorUI.h"
#include <misc/cpp/imgui_stdlib.h> 

#include <imgui.h>
#include <fstream>
#include <memory>
#include <cstring>
#include <filesystem>

void renderAudioInspector(std::shared_ptr<AudioComponent> audio) {
    if (!audio) {
        ImGui::Text("No audio selected.");
        return;
    }

    // Editable name
    ImGui::InputText("Name", &audio->name);

    // Editable audio file path
    ImGui::InputText("Audio File", &audio->audioPath);

    // Editable volume
    ImGui::SliderFloat("Volume", &audio->volume, 0.0f, 1.0f);

    // Editable loop
    ImGui::Checkbox("Loop", &audio->loop);

    // Save button
    if (ImGui::Button("Save")) {
        std::string filePath = audio->name + ".json";
        std::ofstream file(filePath);
        if (file.is_open()) {
            file << audio->toJson().dump(4);
            file.close();
            ImGui::Text("Saved to %s", filePath.c_str());
        } else {
            ImGui::Text("Failed to save to %s", filePath.c_str());
        }
    }
}