#pragma once

#include <imgui.h>
#include <cstring>
#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/Components/ChoiceComponent.hpp"

inline void renderChoiceInspector(const std::shared_ptr<ChoiceComponent>& comp) {
    ImGui::Text("Choice Options:");
    for (size_t i = 0; i < comp->options.size(); ++i) {
        ImGui::PushID(static_cast<int>(i));
        char buffer[256];
        std::strncpy(buffer, comp->options[i].text.c_str(), sizeof(buffer));
        buffer[255] = '\0';

        if (ImGui::InputText("Text", buffer, sizeof(buffer))) {
            comp->options[i].text = buffer;
        }

        ImGui::Text("Trigger: %d", static_cast<int>(comp->options[i].trigger));
        ImGui::Separator();
        ImGui::PopID();
    }

    if (ImGui::Button("Add Option")) {
        comp->options.push_back({"New choice", ComponentType::Unknown});
    }
}