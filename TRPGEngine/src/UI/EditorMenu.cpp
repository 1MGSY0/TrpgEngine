#include "EditorUI.hpp"
#include "Application.hpp"

#include "UI/AssetPanels/CharacterPanel.hpp"
#include "UI/AssetPanels/ScriptPanel.hpp"

#include "Resources/ResourceManager.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/ComponentRegistry.hpp"
#include "Project/ProjectManager.hpp"
#include "Project/BuildSystem.hpp"
#include "ImGUIUtils/ImGuiUtils.hpp"
#include "Templates/EntityTemplates.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <json.hpp>
#include <filesystem>
#include <cstring>

using json = nlohmann::json;

void EditorUI::renderMenuBar() {
    if (ImGui::BeginMenuBar()) {

        // ---------------- FILE MENU ----------------
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
                                forceFolderRefresh();
                            }
                        }
                    }
                    ImGui::EndMenu(); // Assets
                }
                ImGui::EndMenu(); // Import
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
                            setStatusMessage(success ? "Build successful." : "Build failed.");
                        }
                    }
                }
                ImGui::EndMenu(); // Export
            }

            ImGui::EndMenu(); // File
        }

        // ---------------- EDIT MENU ----------------
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Rename Selected File")) {
                showRenamePopup = true;
                strcpy(newName, m_selectedAssetName.c_str());
                ImGui::OpenPopup("RenameAssetPopup");
            }
            ImGui::EndMenu();
        }

        // ---------------- ENTITY MENU ----------------
        if (ImGui::BeginMenu("Entities")) {
            if (ImGui::MenuItem("New Entity...")) {
                ImGui::OpenPopup("NewEntityPopup");
            }

            if (ImGui::BeginMenu("Add Component to Selected")) {
                Entity selected = getSelectedEntity();
                if (selected != INVALID_ENTITY) {
                    for (const auto& info : ComponentTypeRegistry::getAllInfos()) {
                        if (ImGui::MenuItem(info.key.c_str())) {
                            EntityManager::get().addComponent(selected, info.factory());
                            setStatusMessage("Added component: " + info.key);
                        }
                    }
                } else {
                    ImGui::TextDisabled("No entity selected");
                }
                ImGui::EndMenu(); // Add Component
            }

            if (ImGui::MenuItem("Save Selected Entity as .entity")) {
                Entity selected = getSelectedEntity();
                if (selected != INVALID_ENTITY) {
                    json j = EntityManager::get().serializeEntity(selected);
                    ResourceManager::get().saveAssetFile(j, "entity_" + std::to_string(selected), ".entity");
                    setStatusMessage("Saved selected entity.");
                }
            }

            ImGui::EndMenu(); // Entities
        }

        // ---------------- SELECT MENU ----------------
        if (ImGui::BeginMenu("Select")) {
            if (ImGui::MenuItem("Select All")) {
                // TODO: implement selection
            }
            ImGui::EndMenu();
        }

        // ---------------- VIEW MENU ----------------
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Appearance")) {
                // TODO: implement theme toggle
            }
            ImGui::EndMenu();
        }

        // ---------------- RUN MENU ----------------
        if (ImGui::BeginMenu("Run")) {
            if (ImGui::MenuItem("Play", nullptr, m_app->isPlaying())) {
                m_app->togglePlayMode();
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    // ---------------- POPUPS ----------------
    if (showRenamePopup) {
        ImGui::OpenPopup("RenameAssetPopup");
        showRenamePopup = false; // Reset immediately after opening
    }

    // Same for new entity popup
    if (showNewEntityPopup) {
        ImGui::OpenPopup("NewEntityPopup");
        showNewEntityPopup = false;
    }

    // Now call render logic
    renderRenamePopup();
    renderNewEntityPopup();
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


void EditorUI::renderRenamePopup() {
    if (ImGui::BeginPopupModal("RenameAssetPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Rename Asset");
        ImGui::Separator();

        ImGui::InputText("New Name", newName, IM_ARRAYSIZE(newName));

        if (ImGui::Button("Rename")) {
            if (strlen(newName) > 0) {
                auto* editor = EditorUI::get();
                if (editor) {
                    std::filesystem::path folder = editor->getSelectedFolder();
                    std::filesystem::path oldPath = folder / m_selectedAssetName;
                    std::string newNameStr = newName;

                    if (ResourceManager::get().renameAssetFile(oldPath.string(), newNameStr)) {
                        m_selectedAssetName = newNameStr;
                        setStatusMessage("Asset renamed to " + newNameStr);
                    } else {
                        setStatusMessage("Rename failed.");
                    }
                }

                strcpy(newName, "");
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

void EditorUI::renderNewEntityPopup() {
    static int selectedTemplate = 0;
    static char nameBuffer[128] = "";

    if (ImGui::BeginPopupModal("NewEntityPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        const auto& templates = getEntityTemplates();

        ImGui::Text("Choose Entity Type:");
        ImGui::Separator();
        for (int i = 0; i < templates.size(); ++i) {
            if (ImGui::Selectable(templates[i].name.c_str(), selectedTemplate == i)) {
                selectedTemplate = i;
            }
        }

        ImGui::InputText("File Name", nameBuffer, IM_ARRAYSIZE(nameBuffer));

        ImGui::Separator();
        if (ImGui::Button("Create & Save")) {
            Entity e = EntityManager::get().createEntity();
            for (const auto& comp : templates[selectedTemplate].components)
                EntityManager::get().addComponent(e, comp);

            setSelectedEntity(e);

            // Save to disk
            json j = EntityManager::get().serializeEntity(e);
            std::string fileName = std::string(nameBuffer) + ".entity";
            ResourceManager::get().saveAssetFile(j, nameBuffer, ".entity");

            EditorUI::get()->setStatusMessage("Created and saved: " + std::string(nameBuffer));
            ImGui::CloseCurrentPopup();
            nameBuffer[0] = '\0'; // reset
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
            nameBuffer[0] = '\0';
        }

        ImGui::EndPopup();
    }
}