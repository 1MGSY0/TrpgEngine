#include "ScenePanel.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/TransformComponent.hpp"
#include <imgui_internal.h>
#include <glad/glad.h>
#include <iostream>
#include "../../../../packages/stb/stb_image.h"

ScenePanel::ScenePanel() {}

ScenePanel::~ScenePanel() {}

GLuint fbo = 0, fboTexture = 0, rbo = 0;
int fbWidth = 800, fbHeight = 600;

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

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fbWidth, fbHeight);
    glEnable(GL_DEPTH_TEST);

    // Clear framebuffer
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: Draw your 3D object here
    // yourShader.use();
    // Set uniforms (MVP)
    // yourModel.draw(yourShader);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initSceneFrameBuffer(){
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Texture
    glGenTextures(1, &fboTexture);
    glBindTexture(GL_TEXTURE_2D, fboTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fbWidth, fbHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

    // Renderbuffer (depth/stencil)
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fbWidth, fbHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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