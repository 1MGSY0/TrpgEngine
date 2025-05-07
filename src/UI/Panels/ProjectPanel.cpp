#include "ProjectPanel.h"
#include "Project/ProjectManager.h"
#include "Project/ResourceManager.h"
#include "ImGUIUtils/ImGuiUtils.h"

#include <imgui.h>
#include <string>

void ProjectPanel::render() {
    static std::string status;

    if (ImGui::Button("New Project")) {
        if (ResourceManager::get().hasUnsavedChanges()) {
            // Leave unsaved warning logic to EditorUI (centralized)
        } else {
            ResourceManager::get().clear();
            ProjectManager::setCurrentProjectPath("");  // New project, no saved path yet
            ResourceManager::get().setUnsavedChanges(true);
            status = "New project created.";
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Load Project")) {
        std::string path = openFileDialog();
        if (!path.empty()) {
            if (ProjectManager::loadProject(path)) {
                status = "Project loaded.";
            } else {
                status = "Failed to load project.";
            }
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Save Project")) {
        std::string path = ProjectManager::getCurrentProjectPath();

        // First-time save, no path yet
        if (path.empty()) {
            path = saveFileDialog();
            if (path.empty()) return; // Cancelled
            ProjectManager::setCurrentProjectPath(path);
        }

        if (ProjectManager::save()) {
            ResourceManager::get().setUnsavedChanges(false);
            status = "Project saved.";
        } else {
            status = "Save failed.";
        }
    }

    if (!status.empty()) {
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.5f, 1.0f), "%s", status.c_str());
    }
}
