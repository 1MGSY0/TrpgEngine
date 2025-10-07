#include "EditorUI.hpp"
#include <iostream>

using json = nlohmann::json;

void EditorUI::renderMenuBar() {
    if (ImGui::BeginMenuBar()) {

        // ---------------- FILE MENU ----------------
        if (ImGui::BeginMenu("Project")) {
            if (ImGui::MenuItem("New Project")) {
                std::cout << "[EditorUI] Menu: New Project clicked\n";
                if (ResourceManager::get().hasUnsavedChanges()) {
                    std::cout << "[EditorUI] Unsaved changes -> opening Unsaved Changes modal\n";
                    ImGui::OpenPopup("Unsaved Changes");
                    m_showUnsavedPrompt = true;
                    m_actionAfterPrompt = true; // means: do New Project after prompt
                } else {
                    std::cout << "[EditorUI] No unsaved changes -> creating default project\n";
                    ResourceManager::get().clear();
                    EntityManager::get().clear();
                    ProjectManager::setCurrentProjectPath("");
                    std::error_code ec;
                    std::filesystem::create_directories("Runtime", ec);
                    if (ec) std::cout << "[EditorUI] create_directories(Runtime) error: " << ec.message() << "\n";

                    const std::string name = "Untitled";
                    const std::string path = "Runtime/" + name + ".trpgproj";
                    std::cout << "[EditorUI] CreateNewProject(name=" << name << ", path=" << path << ")\n";
                    if (!ProjectManager::CreateNewProject(name, path)) {
                        std::cout << "[EditorUI] CreateNewProject FAILED\n";
                    } else {
                        ProjectManager::setCurrentProjectPath(path);
                        ResourceManager::get().setUnsavedChanges(true);
                        std::cout << "[EditorUI] New project created -> requestProjectInfoPrompt()\n";
                        ProjectManager::requestProjectInfoPrompt();   // deferred-safe
                    }
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

            if (ImGui::MenuItem("Scene Metadata Settings...")) {
                showProjectMetaPopup = true;
                ImGui::OpenPopup("ProjectMetaPopup");
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
                showNewEntityPopup = true;
                ImGui::OpenPopup("NewEntityPopup");
            }

            if (ImGui::BeginMenu("Add Component to Selected")) {
                Entity selected = getSelectedEntity();
                if (selected != INVALID_ENTITY) {
                    for (const auto& [type, info] : ComponentTypeRegistry::getAllInfos()) {
                        if (ImGui::MenuItem(info.key.c_str())) {
                            auto component = info.loader(nlohmann::json{}); 
                            EntityManager::get().addComponent(selected, component);
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

    // (PROJECT INFO)
    if (ProjectManager::consumeProjectInfoPrompt()) {
        std::cout << "[EditorUI] consumeProjectInfoPrompt() = true, will OpenPopup(ProjectMetaPopup)\n";
        openProjectInfoPopupOnce(); // flips showProjectMetaPopup
    }

    if (showProjectMetaPopup) {
        ImGui::OpenPopup("ProjectMetaPopup");
        showProjectMetaPopup = false;
    }

    // Now call render logic
    renderRenamePopup();
    renderNewEntityPopup();
    renderProjectMetaPopup(ProjectManager::getProjectMetaEntity());
}

void EditorUI::renderProjectMetaPopup(Entity metaEntity) {

    if (ImGui::BeginPopupModal("ProjectMetaPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        auto& em = EntityManager::get();
        std::cout << "[EditorUI] ProjectMetaPopup: metaEntity=" << (int)metaEntity << "\n";

        auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
        if (!base) {
            std::cout << "[EditorUI] ProjectMetaPopup: component missing -> creating now\n";
            auto metaComp = std::make_shared<ProjectMetaComponent>();
            auto res = em.addComponent(metaEntity, metaComp);
            std::cout << "[EditorUI] ProjectMetaPopup: addComponent result=" << (int)res << "\n";
            base = metaComp; // continue below with a valid component
        }

        auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);

        ImGui::Text("Project Metadata");
        ImGui::Separator();

        // Project Name
        char nameBuffer[128];
        strncpy(nameBuffer, meta->projectName.c_str(), sizeof(nameBuffer));
        nameBuffer[127] = '\0';
        if (ImGui::InputText("Project Name", nameBuffer, sizeof(nameBuffer))) {
            meta->projectName = nameBuffer;
        }

        // Author
        char authorBuffer[128];
        strncpy(authorBuffer, meta->author.c_str(), sizeof(authorBuffer));
        authorBuffer[127] = '\0';
        if (ImGui::InputText("Author", authorBuffer, sizeof(authorBuffer))) {
            meta->author = authorBuffer;
        }

        // Version
        char versionBuffer[64];
        strncpy(versionBuffer, meta->version.c_str(), sizeof(versionBuffer));
        versionBuffer[63] = '\0';
        if (ImGui::InputText("Version", versionBuffer, sizeof(versionBuffer))) {
            meta->version = versionBuffer;
        }

        ImGui::Checkbox("Project Active", &meta->isActive);
        ImGui::Separator();

        ImGui::Text("Scenes in Project (FlowNodes)");
        ImGui::TextDisabled("(Each scene is a FlowNode entity)");
        ImGui::Separator();

        Entity currentStart = meta->startNode;

        for (size_t i = 0; i < meta->sceneNodes.size(); ++i) {
            Entity nodeId = meta->sceneNodes[i];
            auto flowComp = em.getComponent(nodeId, ComponentType::FlowNode);

            if (!flowComp) {
                ImGui::BulletText("Scene %zu: [Missing FlowNode] (ID: %u)", i, nodeId);
                continue;
            }

            auto flow = std::static_pointer_cast<FlowNodeComponent>(flowComp);
            bool isStart = (nodeId == currentStart);

            ImGui::Text("Scene %zu: %s (ID: %u)%s", i, flow->name.c_str(), nodeId, isStart ? " [Start Scene]" : "");

            if (!isStart && ImGui::Button(("Set as Start##" + std::to_string(i)).c_str())) {
                meta->startNode = nodeId;
            }

            ImGui::Separator();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextDisabled("Note: Project = ProjectMetaComponent. Scenes = FlowNodes.");

        if (ImGui::Button("Save")) {
            std::cout << "[EditorUI] ProjectMetaPopup: Save\n";
            // TODO: write back fields to meta + mark unsaved
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Close")) {
            std::cout << "[EditorUI] ProjectMetaPopup: Close\n";
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
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
    static int selectedTypeIndex = 0;
    static char nameBuffer[128] = "";

    const auto typeNames = getEntityTypeNames();
    const auto& templateMap = getEntityTemplateMap();

    static bool shouldClosePopup = false;

    if (ImGui::BeginPopupModal("NewEntityPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        shouldClosePopup = false;

        ImGui::Text("Choose Entity Type:");
        ImGui::Separator();

        ImGui::BeginChild("TypeList", ImVec2(200, 150), true);
        for (int i = 0; i < typeNames.size(); ++i) {
            ImGui::PushID(i);
            bool isSelected = (selectedTypeIndex == i);
            if (ImGui::Selectable(typeNames[i].second.c_str(), isSelected)) {
                selectedTypeIndex = i;
                std::cout << "Selected entity type: " << typeNames[i].second << std::endl;
            }
            if (isSelected)
                ImGui::SetItemDefaultFocus();
            ImGui::PopID();
        }
        ImGui::EndChild();

        ImGui::InputText("Entity Name", nameBuffer, IM_ARRAYSIZE(nameBuffer));
        ImGui::Separator();

        if (ImGui::Button("Create & Save")) {
            if (strlen(nameBuffer) == 0) {
                EditorUI::get()->setStatusMessage("Entity name cannot be empty.");
                std::cout << "[ERROR] Entity name is empty.\n";
            } else {
                EntityType type = typeNames[selectedTypeIndex].first;
                Entity e = EntityManager::get().createEntity(EntityManager::get().getSelectedEntity());
                std::cout << "Created new entity with ID: " << e << std::endl;

                EntityManager::get().setEntityMeta(e, nameBuffer, type);
                std::cout << "Entity metadata set. Name: " << nameBuffer << ", Type: " << static_cast<int>(type) << std::endl;

                auto it = templateMap.find(type);
                if (it != templateMap.end()) {
                    size_t added = 0;
                    for (const auto& make : it->second.factories) {
                        if (!make) { std::cout << "[ERROR] Null factory in template!\n"; continue; }
                        auto comp = make();                                   // ← fresh instance
                        if (!comp) { std::cout << "[ERROR] Factory returned null!\n"; continue; }
                        EntityManager::get().addComponent(e, comp);
                        ++added;
                    }
                    std::cout << "Added " << added << " components.\n";
                } else {
                    std::cout << "[WARNING] No template found for this type.\n";
                }

                std::string fileName = ProjectManager::getCurrentProjectPath();
                bool saved = ProjectManager::saveProjectToFile(fileName);

                if (!saved) {
                    std::cout << "[ERROR] Failed to save entity.\n";
                    setStatusMessage("Failed to save project file.");
                } else {
                    std::cout << "Saved project file as: " << fileName << std::endl;
                    setSelectedEntity(e);
                    setStatusMessage("Created and saved: " + std::string(nameBuffer));
                    shouldClosePopup = true;
                    nameBuffer[0] = '\0';
                }
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            std::cout << "New entity creation cancelled.\n";
            nameBuffer[0] = '\0';
            shouldClosePopup = true;
        }

        if (shouldClosePopup)
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }
}