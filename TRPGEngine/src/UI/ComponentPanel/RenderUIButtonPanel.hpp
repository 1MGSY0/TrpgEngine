#pragma once

#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/Components/UIButtonComponent.hpp"

inline void renderUIButtonInspector(const std::shared_ptr<UIButtonComponent>& btn) {
    if (!btn) return;

    ImGui::InputText("Button Text", &btn->text);

    ImGui::InputText("Font Path", &btn->fontPath);
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH")) {
            btn->fontPath = static_cast<const char*>(payload->Data);
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::InputText("Background Image", &btn->imagePath);
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH")) {
            btn->imagePath = static_cast<const char*>(payload->Data);
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::InputText("Target FlowNode", &btn->targetFlowNode);
    ImGui::TextDisabled("Target is optional. If set, clicking this button advances the flow.");
}