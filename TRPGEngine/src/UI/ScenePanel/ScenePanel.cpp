#include "ScenePanel.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/TransformComponent.hpp"
#include <imgui_internal.h>

ScenePanel::ScenePanel() {}

ScenePanel::~ScenePanel() {}

void ScenePanel::renderScenePanel() {
    m_panelSize = ImGui::GetContentRegionAvail();
    m_origin = ImGui::GetCursorScreenPos();

    // Draw background
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(
        m_origin,
        ImVec2(m_origin.x + m_panelSize.x, m_origin.y + m_panelSize.y),
        IM_COL32(50, 50, 50, 255)
    );

    for (Entity entity : EntityManager::get().getAllEntities()) {
        if (EntityManager::get().hasComponent(entity, ComponentType::Transform))
            renderEntity(entity);
    }

    handleDragDrop();
}

void ScenePanel::renderEntity(Entity entity) {
    auto transformBase = EntityManager::get().getComponent(entity, ComponentType::Transform);
    if (!transformBase) return;

    auto transform = std::static_pointer_cast<TransformComponent>(transformBase);
    ImVec2 pos = ImVec2(m_origin.x + transform->position.x, m_origin.y + transform->position.y);
    ImVec2 size = transform->size;


    // Resize logic
    ImGui::SetCursorScreenPos(ImVec2(pos.x + size.x - 10, pos.y + size.y - 10));
    ImGui::InvisibleButton(("##resize" + std::to_string(entity)).c_str(), ImVec2(10, 10));

    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        transform->size.x += ImGui::GetIO().MouseDelta.x;
        transform->size.y += ImGui::GetIO().MouseDelta.y;
    }
}

void ScenePanel::handleDragDrop() {
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_CHARACTER")) {
            Entity droppedEntity = *(Entity*)payload->Data;

            // Ensure Transform exists
            if (!EntityManager::get().hasComponent(droppedEntity, ComponentType::Transform)) {
                auto transform = std::make_shared<TransformComponent>();
                EntityManager::get().addComponent(droppedEntity, transform);
            }
        }
        ImGui::EndDragDropTarget();
    }
}

void ScenePanel::renderTextOverlay(const DialogueComponent& text) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 textPos = ImVec2(m_origin.x + 20, m_origin.y + 20);
    const float lineSpacing = 22.0f;

    for (const auto& line : text.lines) {
        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), line.c_str());
        textPos.y += lineSpacing;
    }
}