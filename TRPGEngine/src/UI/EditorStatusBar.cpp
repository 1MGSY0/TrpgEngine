#include "EditorUI.h"
#include <imgui.h>

void EditorUI::renderStatusBar() {
    static float timeSinceStatus = 0.0f;

    if (!m_saveStatus.empty()) {
        timeSinceStatus += ImGui::GetIO().DeltaTime;
        if (timeSinceStatus > 5.0f) {
            m_saveStatus.clear();
            timeSinceStatus = 0.0f;
            return;
        }
    } else {
        return;
    }

    ImVec2 windowSize = ImGui::GetWindowSize();
    float statusHeight = ImGui::GetTextLineHeightWithSpacing() + 10.0f;

    ImGui::SetCursorPosY(windowSize.y - statusHeight);
    ImGui::Separator();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);

    ImGui::BeginChild("StatusBar", ImVec2(0, statusHeight), false,
                      ImGuiWindowFlags_NoScrollbar |
                      ImGuiWindowFlags_NoScrollWithMouse |
                      ImGuiWindowFlags_NoDecoration);

    ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.4f, 1.0f), "%s", m_saveStatus.c_str());
    ImGui::EndChild();
}