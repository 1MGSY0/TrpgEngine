#pragma once

#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/Components/ChoiceComponent.hpp"

inline void renderChoiceInspector(const std::shared_ptr<ChoiceComponent>& comp) {
    ImGui::Text("Choice Options:");
    for (size_t i = 0; i < comp->options.size(); ++i) {
        char buffer[256];
        strncpy(buffer, comp->options[i].text.c_str(), sizeof(buffer));
        buffer[255] = '\0';

        if (ImGui::InputText(("Option " + std::to_string(i)).c_str(), buffer, sizeof(buffer))) {
            comp->options[i].text = buffer;
        }

        ImGui::Text("Trigger: %d", static_cast<int>(comp->options[i].trigger));
        ImGui::Separator();
    }

    if (ImGui::Button("Add Option")) {
        comp->options.push_back({"New choice", ComponentType::Unknown});
    }
}