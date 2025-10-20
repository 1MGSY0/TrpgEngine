#pragma once

#include <imgui.h>
#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/Components/BackgroundComponent.hpp"

inline void renderBackgroundInspector(const std::shared_ptr<BackgroundComponent>& comp) {
    char buffer[256];
    strncpy(buffer, comp->assetPath.c_str(), sizeof(buffer));
    buffer[255] = '\0';

    if (ImGui::InputText("Asset Path", buffer, sizeof(buffer))) {
        comp->assetPath = buffer;
    }

    // Support drag and drop from file browser
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH")) {
            const char* droppedPath = static_cast<const char*>(payload->Data);
            comp->assetPath = droppedPath;
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::TextWrapped("Enter a relative or absolute path to the background texture (e.g., 'Assets/Backgrounds/scene1.png').");
}