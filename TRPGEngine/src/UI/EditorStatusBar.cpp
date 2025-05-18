#include "EditorUI.hpp"
#include <imgui.h>

void EditorUI::setStatusMessage(const std::string& message) {
    m_saveStatus = message;
    m_statusTimer = 0.0f;  // Reset timer when setting new message
}

void EditorUI::renderStatusBar() {
    if (m_saveStatus.empty()) return;

    m_statusTimer += ImGui::GetIO().DeltaTime;
    if (m_statusTimer > 5.0f) {
        m_saveStatus.clear();
        m_statusTimer = 0.0f;
        return;
    }

    // This must match the name used in DockBuilderDockWindow("StatusBar", ...)
    if (ImGui::Begin("StatusBar", nullptr,
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoScrollbar)) {

        ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.4f, 1.0f), "%s", m_saveStatus.c_str());
    }

    ImGui::End();
}
