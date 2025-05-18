#include "EditorUI.hpp"

#include "UI/AssetPanels/CharacterPanel.hpp"
#include "UI/AssetPanels/ScriptPanel.hpp"

#include "Resources/ResourceManager.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/ComponentRegistry.hpp"
#include "Project/ProjectManager.hpp"
#include "Project/BuildSystem.hpp"
#include "ImGUIUtils/ImGuiUtils.hpp"
#include <imgui.h>
#include <filesystem>
#include <json.hpp>

using json = nlohmann::json;

void EditorUI::renderMenuBar() {
    if (ImGui::BeginMenuBar()) {
        // --- FILE MENU ---
        if (ImGui::BeginMenu("File")) {

            if (ImGui::MenuItem("New Project")) {
                if (ResourceManager::get().hasUnsavedChanges()) {
                    ImGui::OpenPopup("Unsaved Changes");
                    m_showUnsavedPrompt = true;
                    m_actionAfterPrompt = true;
                } else {
                    ResourceManager::get().clear();
                    EntityManager::get().clear();
                    ProjectManager::setCurrentProjectPath("");
                    ResourceManager::get().setUnsavedChanges(true);
                }
            }

            if (ImGui::MenuItem("Open Project...")) {
                std::string path = openFileDialog();
                if (!path.empty()) {
                    if (ResourceManager::get().hasUnsavedChanges()) {
                        m_showUnsavedPrompt = true;
                        m_actionAfterPrompt = false;
                        ProjectManager::setTempLoadPath(path);
                    } else {
                        ProjectManager::loadProject(path);
                    }
                }
            }

            if (ImGui::MenuItem("Save Project")) {
                std::string path = ProjectManager::getCurrentProjectPath();
                if (path.empty()) path = saveFileDialog();

                if (!path.empty()) {
                    ProjectManager::setCurrentProjectPath(path);
                    ProjectManager::save();
                    ResourceManager::get().setUnsavedChanges(false);
                }
            }

            if (ImGui::MenuItem("Save Project As...")) {
                std::string path = saveFileDialog();
                if (!path.empty()) {
                    ProjectManager::saveProjectToFile(path);
                    ProjectManager::setCurrentProjectPath(path);
                    ResourceManager::get().setUnsavedChanges(false);
                }
            }

            if (ImGui::BeginMenu("Import")) {
                if (ImGui::BeginMenu("Assets")) {
                    for (const FileAssetTypeInfo& asset : getAllFileAssetTypes()) {
                        if (ImGui::MenuItem(asset.name.c_str())) {
                            std::string filter = convertExtensionsToFilter(asset.extensions);
                            std::string path = openFileDialog(filter.c_str());
                            if (!path.empty()) {
                                ResourceManager::get().importFileAsset(path, asset.type);
                                EditorUI::get()->forceFolderRefresh();
                            }
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Export")) {
                if (ImGui::MenuItem("Build Project")) {
                    std::string path = ProjectManager::getCurrentProjectPath();
                    if (path.empty()) {
                        ImGui::OpenPopup("Missing Project Path");
                    } else {
                        std::string out = openFileDialog();
                        if (!out.empty()) {
                            bool success = BuildSystem::buildProject(path, out);
                            m_saveStatus = success ? "Build successful." : "Build failed.";
                        }
                    }
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        // --- EDIT MENU ---
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

            // --- ENTITY MENU ---
            if (ImGui::BeginMenu("Entities")) {

                if (ImGui::MenuItem("New Entity")) {
                    Entity e = EntityManager::get().createEntity();
                    setSelectedEntity(e);
                }

                if (ImGui::BeginMenu("Add Component to Selected")) {
                    Entity selected = getSelectedEntity();
                    if (selected != INVALID_ENTITY) {
                        for (const auto& info : ComponentTypeRegistry::getAllInfos()) {
                            if (ImGui::MenuItem(info.key.c_str())) {
                                auto comp = info.factory();
                                EntityManager::get().addComponent(selected, comp);
                            }
                        }
                    } else {
                        ImGui::TextDisabled("No entity selected");
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::MenuItem("Save Selected Entity as .entity")) {
                    Entity selected = getSelectedEntity();
                    if (selected != INVALID_ENTITY) {
                        json j = EntityManager::get().serializeEntity(selected);
                        ResourceManager::get().saveAssetFile(j, "entity_" + std::to_string(selected), ".entity");
                    }
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
