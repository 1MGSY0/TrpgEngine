#include "EditorUI.hpp"

#include "UI/AssetPanels/CharacterPanel.hpp"
#include "UI/AssetPanels/ScriptPanel.hpp"

#include "Resources/ResourceManager.hpp"
#include "Project/ProjectManager.hpp"
#include "Project/BuildSystem.hpp"
#include "ImGUIUtils/ImGuiUtils.hpp"
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
                    for (const auto& info : getAllFileAssetTypes()) {
                        if (ImGui::MenuItem(info.name.c_str())) {
                            std::string path = openFileDialog(info.extensions); // pass filters
                            if (!path.empty()) {
                                ResourceManager::get().importFileAsset(path, info.type);
                                if (auto* editor = EditorUI::get()) editor->forceFolderRefresh();
                            }
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
            if (ImGui::MenuItem("Rename Selected File")) {
                static char newName[128] = "";
                std::string currentName = m_selectedAssetName;
                ImGui::OpenPopup("RenameAssetPopup");

                if (ImGui::BeginPopupModal("RenameAssetPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::InputText("New Name", newName, IM_ARRAYSIZE(newName));

                    if (ImGui::Button("Rename")) {
                        if (strlen(newName) > 0) {
                            ResourceManager::get().renameAssetFile(currentName, newName);
                            strcpy(newName, "");
                            m_selectedAssetName = newName;
                            ImGui::CloseCurrentPopup();
                        }
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Cancel")) {
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }
            }
            if (ImGui::BeginMenu("Entities")) {
                if (ImGui::MenuItem("New Entity")) {
                    Entity entity = g_entityManager->createEntity();  // Global or passed context
                    g_editorUI->setSelectedEntity(entity);            // For inspector focus
                    g_entityManager->addComponent(entity, std::make_shared<TransformComponent>());  // default
                }

                if (ImGui::BeginMenu("Add Component to Entity")) {
                    Entity entity = g_editorUI->getSelectedEntity();
                    if (entity != INVALID_ENTITY) {
                        for (const auto& info : ComponentTypeRegistry::getAllInfos()) {
                            if (ImGui::MenuItem(info.key.c_str())) {
                                auto comp = info.factory();
                                g_entityManager->addComponent(entity, comp);
                            }
                        }
                    }
                    ImGui::EndMenu();
                }
                Entity selected = getSelectedEntity();
                if (selected != INVALID_ENTITY) {
                    std::string filename = "MyEntity";
                    json j = EntitySerializer::serialize(selected, *g_entityManager);
                    ResourceManager::get().saveAssetFile(j, filename, ".entity");
                }
                ImGui::EndMenu();
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
