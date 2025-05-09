#include "EditorUI.h"
#include "Engine/Resources/ResourceManager.h"
#include "Project/ProjectManager.h"
#include "Project/BuildSystem.h"
#include "ImGUIUtils/ImGuiUtils.h"
#include <imgui.h>

void EditorUI::renderMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New")) {
                if (ResourceManager::get().hasUnsavedChanges()) {
                    ImGui::OpenPopup("Unsaved Changes");
                    m_showUnsavedPrompt = true;
                    m_actionAfterPrompt = true;
                } else {
                    ResourceManager::get().clear();
                    ProjectManager::setCurrentProjectPath(""); 
                    ResourceManager::get().setUnsavedChanges(true);
                }
            }

            if (ImGui::MenuItem("Open...")) {
                std::string file = openFileDialog();
                if (!file.empty()) {
                    if (ResourceManager::get().hasUnsavedChanges()) {
                        m_showUnsavedPrompt = true;
                        m_actionAfterPrompt = false;
                        ProjectManager::setTempLoadPath(file);  // <-- Store path to use after prompt
                    } else {
                        ProjectManager::loadProject(file);
                    }
                }
            }

            if (ImGui::MenuItem("Save")) {
                std::string path = ProjectManager::getCurrentProjectPath();
                if (path.empty()) {
                    path = saveFileDialog();
                    if (path.empty()) {
                        // just skip save, do nothing
                    } else {
                        ProjectManager::setCurrentProjectPath(path);
                        ProjectManager::save();
                        ResourceManager::get().setUnsavedChanges(false);
                    }
                }
            }

            if (ImGui::MenuItem("Save As...")) {
                std::string file = saveFileDialog();
                if (!file.empty()) {
                    ProjectManager::saveProjectToFile(file);  // use the full path
                    ProjectManager::setCurrentProjectPath(file);
                    ResourceManager::get().setUnsavedChanges(false);
                }
            }
            
            if (ImGui::BeginPopupModal("Missing Project Path", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("You must save the project before building.\n\n");
                ImGui::Separator();
                if (ImGui::Button("OK")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            if (ImGui::BeginMenu("Import")) {
                if (ImGui::BeginMenu("Assets")) {
                    if (ImGui::MenuItem("Text")) {
                        std::string path = openFileDialog();
                        if (!path.empty()) {
                            ResourceManager::get().importAssetFromFile(path, AssetType::Text);
                        }
                    }
                    if (ImGui::MenuItem("Character")) {
                        std::string path = openFileDialog();
                        if (!path.empty()) {
                            ResourceManager::get().importAssetFromFile(path, AssetType::Character);
                        }
                    }
                    if (ImGui::MenuItem("Audio")) {
                        std::string path = openFileDialog();
                        if (!path.empty()) {
                            ResourceManager::get().importAssetFromFile(path, AssetType::Audio);
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Export")) {
                if (ImGui::BeginMenu("Build")) {
                    if (ImGui::MenuItem("Build Project")) {
                        const std::string& projectPath = ProjectManager::getCurrentProjectPath();
                
                        if (projectPath.empty()) {
                            ImGui::OpenPopup("Missing Project Path");
                        } else {
                            std::string exportPath = openFileDialog();
                            if (!exportPath.empty()) {
                                bool success = BuildSystem::buildProject(projectPath, exportPath);
                                m_saveStatus = success ? "Build successful." : "Build failed.";
                            }
                        }
                    }
                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenu(); 
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo")) {
                // Undo logic here
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Select")) {
            if (ImGui::MenuItem("Select All")) {
                // Undo logic here
            }
            ImGui::EndMenu();
        }


        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Appearance")) {
                // Theme toggle or scaling coming soon
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Run")) {
            if (ImGui::MenuItem("Run Project")) {
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

void EditorUI::showUnsavedChangesPopup() {
    if (ImGui::BeginPopupModal("Unsaved Changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("You have unsaved changes.\nSave before continuing?\n\n");
        ImGui::Separator();

        if (ImGui::Button("Save and Continue")) {
            std::string dir = ProjectManager::getCurrentProjectPath();
            if (dir.empty()) dir = "Projects/NewProject";
            ProjectManager::saveProjectToFile(dir);
            ResourceManager::get().setUnsavedChanges(false);

            if (m_actionAfterPrompt)
                ResourceManager::get().clear();
            else
                ProjectManager::loadProject(ProjectManager::getTempLoadPath());

            m_showUnsavedPrompt = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Discard Changes")) {
            ResourceManager::get().setUnsavedChanges(false);

            if (m_actionAfterPrompt)
                ResourceManager::get().clear();
            else
                ProjectManager::loadProject(ProjectManager::getTempLoadPath());

            m_showUnsavedPrompt = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            m_showUnsavedPrompt = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

}
