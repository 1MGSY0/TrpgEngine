#include "ScenePanel.hpp"

ScenePanel::ScenePanel() {}

ScenePanel::~ScenePanel() {}

void ScenePanel::renderScenePanel() {
    m_panelSize = ImGui::GetContentRegionAvail();
    m_origin = ImGui::GetCursorScreenPos();

    // Render background
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(m_origin, ImVec2(m_origin.x + m_panelSize.x, m_origin.y + m_panelSize.y), IM_COL32(50, 50, 50, 255));

    // Draw entities
    for (auto& entity : m_entities) {
        renderEntity(entity);
    }

    // Drag & Drop
    handleDragDrop();
}

void ScenePanel::addEntity(std::shared_ptr<Entity> e) {
    m_entities.push_back(e);
}

void ScenePanel::renderEntity(const std::shared_ptr<Entity>& entity) {
    ImVec2 pos = ImVec2(m_origin.x + entity->position.x, m_origin.y + entity->position.y);
    ImVec2 size = entity->size;

    if (entity->textureID) {
        ImGui::GetWindowDrawList()->AddImage(
            (ImTextureID)entity->textureID,
            pos,
            ImVec2(pos.x + size.x, pos.y + size.y)
        );
    }

    // Make resizable via ImGui invisible handle
    ImGui::SetCursorScreenPos(ImVec2(pos.x + size.x - 10, pos.y + size.y - 10));
    ImGui::InvisibleButton(("##resize" + entity->getId()).c_str(), ImVec2(10, 10));
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        entity->size.x += ImGui::GetIO().MouseDelta.x;
        entity->size.y += ImGui::GetIO().MouseDelta.y;
    }
}

void ScenePanel::handleDragDrop() {
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_CHARACTER")) {
            auto entity = *(std::shared_ptr<Entity>*)payload->Data;
            addEntity(entity); // Place in scene
        }
        ImGui::EndDragDropTarget();
    }
}

// Text overlay for dialogues and narration
void ScenePanel::renderTextOverlay(const DialogueComponent& text) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 textPos = ImVec2(m_origin.x + 20, m_origin.y + 20);
    const float lineSpacing = 22.0f; // Adjust based on font size

    for (const auto& line : text.lines) {
        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), line.c_str());
        textPos.y += lineSpacing;
    }
}