// NO SUPPORT FOR CLICKING, OR INTERACTING YET

#include "UI/ScenePanel/ScenePanel.hpp"
#include "Engine/RenderSystem/SceneManager.hpp"
#include "Engine/GameplaySystem/GameInstance.hpp"  // + overlay status

#include <glad/glad.h>
#include <imgui.h>
#include <iostream>

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
        createFramebuffer(static_cast<int>(m_panelSize.x), static_cast<int>(m_panelSize.y));
    }

    // Render scene to framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, static_cast<int>(m_panelSize.x), static_cast<int>(m_panelSize.y));
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.15f, 0.15f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SceneManager::get().renderEditorScene();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Display framebuffer texture in ImGui
    if (m_colorTexture) {
        ImTextureID texID = (ImTextureID)(intptr_t)m_colorTexture;
        ImGui::Image(texID, m_panelSize, ImVec2(0, 1), ImVec2(1, 0));
    }

    // --- Minimal Play overlay (non-interactive placeholder) ---
    if (GameInstance::get().isRunning()) {
        ImVec2 winPos = ImGui::GetItemRectMin();   // top-left of the image
        ImVec2 winSize = ImGui::GetItemRectSize(); // size of the image

        ImGui::SetNextWindowPos(winPos);
        ImGui::SetNextWindowSize(winSize);
        ImGui::SetNextWindowBgAlpha(0.20f);
        ImGuiWindowFlags overlayFlags =
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar;

        if (ImGui::Begin("SceneOverlayHUD", nullptr, overlayFlags)) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.95f, 1.0f, 1.0f));
            ImGui::TextUnformatted("Play Mode");
            ImGui::Separator();
            ImGui::PopStyleColor();

            ImGui::TextDisabled("Dialogue/Choice/Dice UI will appear here.");
            ImGui::TextDisabled("Next: wire to Flow runner for in-editor HUD.");
        }
        ImGui::End();
    }

    ImGui::End();
}