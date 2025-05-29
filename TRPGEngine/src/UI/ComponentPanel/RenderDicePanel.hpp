#pragma once

#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/Components/DiceRollComponent.hpp"

inline void renderDiceInspector(const std::shared_ptr<DiceRollComponent>& comp) {
    ImGui::Text("Dice Roll Settings");

    // Dice sides (e.g. D6, D20)
    ImGui::InputInt("Dice Sides", &comp->sides);

    // Success threshold
    ImGui::InputInt("Success Threshold", &comp->threshold);

    // Flow triggers
    char bufferSuccess[256];
    strncpy(bufferSuccess, comp->onSuccess.c_str(), sizeof(bufferSuccess));
    bufferSuccess[255] = '\0';
    if (ImGui::InputText("On Success Trigger", bufferSuccess, sizeof(bufferSuccess))) {
        comp->onSuccess = bufferSuccess;
    }

    char bufferFailure[256];
    strncpy(bufferFailure, comp->onFailure.c_str(), sizeof(bufferFailure));
    bufferFailure[255] = '\0';
    if (ImGui::InputText("On Failure Trigger", bufferFailure, sizeof(bufferFailure))) {
        comp->onFailure = bufferFailure;
    }

    ImGui::Separator();
    ImGui::TextWrapped("Dice will roll a value between 1 and 'sides'. "
                       "If result >= threshold, OnSuccess will be triggered; otherwise, OnFailure.");
}