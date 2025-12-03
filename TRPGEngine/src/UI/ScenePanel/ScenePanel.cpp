// NO SUPPORT FOR CLICKING, OR INTERACTING YET

#include "UI/ScenePanel/ScenePanel.hpp"
#include "Engine/RenderSystem/SceneManager.hpp"
#include "Engine/GameplaySystem/GameInstance.hpp"  // + overlay status
// + HUD event handling
#include "Engine/GameplaySystem/FlowExecutor.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/DialogueComponent.hpp"
#include "Engine/EntitySystem/Components/ChoiceComponent.hpp"
#include "Engine/EntitySystem/Components/DiceRollComponent.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"
#include "Project/ProjectManager.hpp"

#include <glad/glad.h>
#include <imgui.h>
#include <iostream>
#include "UI/EditorUI.hpp"
#include <random>
#include <algorithm>

ScenePanel::ScenePanel() {
    // Initial framebuffer setup with default size
    createFramebuffer(1280, 720);
}

ScenePanel::~ScenePanel() {
    destroyFramebuffer();
}

void ScenePanel::destroyFramebuffer() {
    if (m_fbo) glDeleteFramebuffers(1, &m_fbo);
    if (m_colorTexture) glDeleteTextures(1, &m_colorTexture);
    if (m_depthRbo) glDeleteRenderbuffers(1, &m_depthRbo);

    m_fbo = 0;
    m_colorTexture = 0;
    m_depthRbo = 0;
}

void ScenePanel::createFramebuffer(int width, int height) {
    destroyFramebuffer(); // Clean up existing buffers

    // Framebuffer
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Color texture
    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);

    // Depth + stencil buffer
    glGenRenderbuffers(1, &m_depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthRbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[ScenePanel] Framebuffer is not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ScenePanel::renderScenePanel() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
    ImGui::Begin("Scene Panel", nullptr, flags);

    ImVec2 contentSize = ImGui::GetContentRegionAvail();
    if (contentSize.x <= 0.0f || contentSize.y <= 0.0f) {
        ImGui::End();
        return;
    }
    if (contentSize.x > 0 && contentSize.y > 0 &&
        (contentSize.x != m_panelSize.x || contentSize.y != m_panelSize.y)) {
        m_panelSize = contentSize;
        // keep existing framebuffer helpers for later; no-op if unneeded
        createFramebuffer(static_cast<int>(m_panelSize.x), static_cast<int>(m_panelSize.y));
    }

    // Prime executor in editor mode so overlay can render events outside runtime
    if (!GameInstance::get().isRunning() && FlowExecutor::get().currentFlowNode() == INVALID_ENTITY) {
        FlowExecutor::get().tick(); // bind to current SceneManager node; no advancement occurs
    }
    const bool previewRunning = (FlowExecutor::get().currentFlowNode() != INVALID_ENTITY);

    // Render SceneManager output directly into a child region so RenderSystem's ImGui draw calls
    // appear inside the Scene Panel.
    ImGui::BeginChild("SceneRenderRegion", m_panelSize, true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    // Report render region for anchoring interactive controls in RenderSystem
    ImVec2 regionMin = ImGui::GetWindowPos();
    ImVec2 regionSize = ImGui::GetWindowSize();
    SceneManager::get().setRenderRegion(regionMin.x, regionMin.y, regionSize.x, regionSize.y);

    // Render scene content (background, models, current event, UI layer)
    if (GameInstance::get().isRunning() || previewRunning) {
        SceneManager::get().renderRuntimeScene();
    } else {
        SceneManager::get().renderEditorScene();
    }

    // Draw an in-panel status pill instead of a floating overlay window (no overlap with editor chrome)
    if (GameInstance::get().isRunning() || previewRunning) {
        auto& em = EntityManager::get();
        Entity currentNode  = SceneManager::get().getCurrentFlowNode();
        auto nodeComp = em.getComponent<FlowNodeComponent>(currentNode);
        int evtIndex = FlowExecutor::get().currentEventIndex();

        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 pad = ImVec2(8, 6);
        ImVec2 textPos = ImVec2(pos.x + 10.0f, pos.y + 8.0f);
        std::string status = std::string("Play Mode  |  Scene: ")
            + (nodeComp ? nodeComp->name : std::string("<Unknown>"))
            + "  |  Event Index: " + std::to_string(evtIndex);

        auto dl = ImGui::GetWindowDrawList();
        ImVec2 textSize = ImGui::CalcTextSize(status.c_str());
        ImVec2 rectMin = ImVec2(textPos.x - pad.x, textPos.y - pad.y);
        ImVec2 rectMax = ImVec2(textPos.x + textSize.x + pad.x, textPos.y + textSize.y + pad.y);

        dl->AddRectFilled(rectMin, rectMax, IM_COL32(20, 24, 28, 180), 6.0f);
        dl->AddRect(rectMin, rectMax, IM_COL32(120, 140, 160, 180), 6.0f);
        dl->AddText(textPos, IM_COL32(220, 230, 240, 255), status.c_str());
    }

    ImGui::EndChild();

    // REMOVE legacy floating overlay to prevent overlapping Flow/Events/Hierarchy tabs
    // (Previously began a window named "SceneOverlayHUD" positioned over the editor)
    // ...deleted: ImGui::Begin("SceneOverlayHUD", ...); entire block with Play Mode text and controls...
    // Note: All interactive UI (Continue/Choice/Roll) is handled by RenderSystem, anchored inside the Scene Panel.

    ImGui::End();
}