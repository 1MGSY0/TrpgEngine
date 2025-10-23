#include "EditorUI.hpp"
#include <iostream>
#include "UI/ImGuiUtils/ImGuiUtils.hpp"
#include <filesystem>
#include <fstream>
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/EntitySystem/Components/DialogueComponent.hpp"
#include "Engine/EntitySystem/Components/ChoiceComponent.hpp"
#include "Engine/EntitySystem/Components/DiceRollComponent.hpp"
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"
#include "Project/ProjectManager.hpp"
#include "UI/FlowPanel/EditorRunControls.hpp"
#include <json.hpp>
#include "Resources/ResourceManager.hpp"
#include "Engine/EntitySystem/ComponentRegistry.hpp"
#include "Engine/RenderSystem/SceneManager.hpp"
#include <algorithm> // + remove/swap

// Helpers for scene/event creation from the Edit menu
namespace {
    // Forward decl (used by getPreferredSceneNode)
    Entity findOwnerScene(Entity evt);

    // Return only the currently selected FlowNode (scene).
    // If an event is selected, infer its owner scene.
    // No fallback to startNode to force explicit user choice when ambiguous.
    Entity getPreferredSceneNode() {
        auto& em = EntityManager::get();
        if (EditorUI* ui = EditorUI::get()) {
            Entity sel = ui->getSelectedEntity();
            if (em.hasComponent(sel, ComponentType::FlowNode))
                return sel;
            // If an event is selected, attach under its owning scene
            if (em.getComponent<DialogueComponent>(sel) ||
                em.getComponent<ChoiceComponent>(sel) ||
                em.getComponent<DiceRollComponent>(sel)) {
                Entity owner = findOwnerScene(sel);
                if (owner != INVALID_ENTITY) return owner;
            }
        }
        return INVALID_ENTITY;
    }

    Entity createSceneFlowNode() {
        auto& em = EntityManager::get();

        Entity metaEntity = ProjectManager::getProjectMetaEntity();
        auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
        if (!base) return INVALID_ENTITY;
        auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);

        Entity e = em.createEntity(INVALID_ENTITY);
        const auto* info = ComponentTypeRegistry::getInfo(ComponentType::FlowNode);
        if (!info || !info->loader) return INVALID_ENTITY;

        std::string defName = "Scene " + std::to_string(meta->sceneNodes.size() + 1);
        nlohmann::json init = nlohmann::json::object();
        init["name"] = defName;
        init["isStart"] = false;
        init["isEnd"] = false;
        init["nextNode"] = -1;
        init["characters"] = nlohmann::json::array();
        init["backgrounds"] = nlohmann::json::array();
        init["uiLayer"] = nlohmann::json::array();
        init["objectLayer"] = nlohmann::json::array();
        init["eventSequence"] = nlohmann::json::array();

        auto comp = info->loader(init);
        if (!comp || em.addComponent(e, comp) != EntityManager::AddComponentResult::Ok) return INVALID_ENTITY;
        comp->Init(e);

        if (auto fn = em.getComponent<FlowNodeComponent>(e)) {
            if (fn->name.empty()) fn->name = defName;
        }

        meta->sceneNodes.push_back(e);

        // If first scene, set as start
        if (meta->startNode == INVALID_ENTITY) {
            meta->startNode = e;
            if (auto fn = em.getComponent<FlowNodeComponent>(e)) fn->isStart = true;
        }

        // Auto-link from currently selected scene if it has no next
        if (EditorUI* ui = EditorUI::get()) {
            Entity selected = ui->getSelectedEntity();
            if (auto selFn = em.getComponent<FlowNodeComponent>(selected)) {
                if (selFn->nextNode < 0) selFn->nextNode = static_cast<int>(e);
            }
            ui->setSelectedEntity(e);
        }
        SceneManager::get().setCurrentFlowNode(e);
        ResourceManager::get().setUnsavedChanges(true);
        return e;
    }

    Entity findOwnerScene(Entity evt) {
        auto& em = EntityManager::get();
        Entity metaEntity = ProjectManager::getProjectMetaEntity();
        auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
        if (!base) return INVALID_ENTITY;
        auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);
        for (Entity n : meta->sceneNodes) {
            auto fn = em.getComponent<FlowNodeComponent>(n);
            if (!fn) continue;
            for (Entity e : fn->eventSequence) if (e == evt) return n;
        }
        return INVALID_ENTITY;
    }

    bool attachEventToScene(Entity evt, Entity sceneNode) {
        auto& em = EntityManager::get();
        if (evt == INVALID_ENTITY || sceneNode == INVALID_ENTITY) return false;
        Entity owner = findOwnerScene(evt);
        if (owner != INVALID_ENTITY) {
            if (auto ownFn = em.getComponent<FlowNodeComponent>(owner)) {
                ownFn->eventSequence.erase(std::remove(ownFn->eventSequence.begin(), ownFn->eventSequence.end(), evt),
                                           ownFn->eventSequence.end());
            }
        }
        if (auto dst = em.getComponent<FlowNodeComponent>(sceneNode)) {
            dst->eventSequence.push_back(evt);
            ResourceManager::get().setUnsavedChanges(true);
            return true;
        }
        return false;
    }

    Entity createEventAndAttach(ComponentType type, Entity sceneNode) {
        auto& em = EntityManager::get();
        const auto* info = ComponentTypeRegistry::getInfo(type);
        if (!info || !info->loader) return INVALID_ENTITY;

        nlohmann::json init = nlohmann::json::object();
        if (type == ComponentType::Dialogue) {
            init["lines"] = nlohmann::json::array();
            init["speaker"] = -1;
            init["advanceOnClick"] = true;
            init["targetFlowNode"] = "";
        } else if (type == ComponentType::Choice) {
            init["options"] = nlohmann::json::array();
        } else if (type == ComponentType::DiceRoll) {
            init["sides"] = 6; init["threshold"] = 1; init["onSuccess"] = ""; init["onFailure"] = "";
        }

        Entity e = em.createEntity(INVALID_ENTITY);
        auto c = info->loader(init);
        if (!c || em.addComponent(e, c) != EntityManager::AddComponentResult::Ok) return INVALID_ENTITY;
        c->Init(e);
        attachEventToScene(e, sceneNode);
        if (EditorUI* ui = EditorUI::get()) ui->setSelectedEntity(e);
        ResourceManager::get().setUnsavedChanges(true);
        return e;
    }

    // Popup state for choosing a scene when none is selected
    static bool s_openChooseScenePopup = false;
    static ComponentType s_pendingEventType = ComponentType::Unknown;
} // anonymous namespace

// Shared Play controls exposed from FlowPlayTester (C-linkage)
extern "C" {
    bool Editor_Run_IsPlaying();
    void Editor_Run_Play();
    void Editor_Run_Stop();
    void Editor_Run_Restart();
}

// Helper: build runtime data.json reflecting scenes, event ownership and targets
static nlohmann::json buildRuntimeDataJson() {
    auto& em = EntityManager::get();
    nlohmann::json root = nlohmann::json::object();
    root["scenes"] = nlohmann::json::array();

    Entity metaEntity = ProjectManager::getProjectMetaEntity();
    auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
    if (!base) return root;
    auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);

    for (Entity nodeId : meta->sceneNodes) {
        auto fn = em.getComponent<FlowNodeComponent>(nodeId);
        if (!fn) continue;

        nlohmann::json scene = nlohmann::json::object();
        scene["id"] = static_cast<uint64_t>(nodeId);
        scene["name"] = fn->name;
        scene["isStart"] = (meta->startNode == nodeId);
        scene["isEnd"] = fn->isEnd;
        scene["nextNode"] = fn->nextNode; // -1 or entity id cast int

        // layers/refs
        auto toArray = [](const std::vector<Entity>& arr) {
            nlohmann::json out = nlohmann::json::array();
            for (Entity e : arr) out.push_back(static_cast<uint64_t>(e));
            return out;
        };
        scene["characters"] = toArray(fn->characters);
        scene["backgrounds"] = toArray(fn->backgroundEntities);
        scene["uiLayer"] = toArray(fn->uiLayer);
        scene["objectLayer"] = toArray(fn->objectLayer);

        // events with ownership and targets
        nlohmann::json events = nlohmann::json::array();
        for (Entity evt : fn->eventSequence) {
            if (evt == INVALID_ENTITY) continue;
            nlohmann::json ev = nlohmann::json::object();
            ev["id"] = static_cast<uint64_t>(evt);
            if (auto d = em.getComponent<DialogueComponent>(evt)) {
                ev["type"] = "Dialogue";
                ev["lines"] = d->lines;
                ev["speaker"] = static_cast<int64_t>(d->speaker);
                ev["advanceOnClick"] = d->advanceOnClick;
                ev["target"] = d->targetFlowNode; // scene name or @Event:id
            } else if (auto c = em.getComponent<ChoiceComponent>(evt)) {
                ev["type"] = "Choice";
                nlohmann::json opts = nlohmann::json::array();
                for (auto& o : c->options) {
                    // Persist full text including " -> target" (editor format)
                    opts.push_back(o.text);
                }
                ev["options"] = opts;
            } else if (auto r = em.getComponent<DiceRollComponent>(evt)) {
                ev["type"] = "DiceRoll";
                ev["sides"] = r->sides;
                ev["threshold"] = r->threshold;
                ev["onSuccess"] = r->onSuccess; // scene name or @Event:id
                ev["onFailure"] = r->onFailure; // scene name or @Event:id
            } else {
                ev["type"] = "Unknown";
            }
            events.push_back(ev);
        }
        scene["events"] = events;

        root["scenes"].push_back(scene);
    }

    return root;
}

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
                const char* projFilter = "TRPG Project (*.trpgproj)\0*.trpgproj\0All Files (*.*)\0*.*\0";
                std::string path = openFileDialog(projFilter);
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
                const char* projFilter = "TRPG Project (*.trpgproj)\0*.trpgproj\0All Files (*.*)\0*.*\0";
                std::string path = ProjectManager::getCurrentProjectPath();
                if (path.empty()) path = saveFileDialog(projFilter);

                if (!path.empty()) {
                    ProjectManager::setCurrentProjectPath(path);
                    ProjectManager::save();
                    ResourceManager::get().setUnsavedChanges(false);
                }
            }

            if (ImGui::MenuItem("Save Project As...")) {
                const char* projFilter = "TRPG Project (*.trpgproj)\0*.trpgproj\0All Files (*.*)\0*.*\0";
                std::string path = saveFileDialog(projFilter);
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
                        const char* anyFilter = "All Files (*.*)\0*.*\0";
                        std::string out = openFileDialog(anyFilter);
                        if (!out.empty()) {
                            bool success = BuildSystem::buildProject(path, out);
                            setStatusMessage(success ? "Build successful." : "Build failed.");
                        }
                    }
                }
                // + Export runtime data.json (scenes, events, targets)
                if (ImGui::MenuItem("Export Runtime Data (data.json)")) {
                    try {
                        std::filesystem::create_directories("Runtime");
                        json j = buildRuntimeDataJson();
                        std::ofstream ofs("Runtime/data.json", std::ios::binary | std::ios::trunc);
                        ofs << j.dump(2);
                        ofs.close();
                        setStatusMessage("Exported Runtime/data.json");
                    } catch (const std::exception& ex) {
                        setStatusMessage(std::string("Export failed: ") + ex.what());
                    }
                }
                ImGui::EndMenu(); // Export
            }

            ImGui::EndMenu(); // File
        }

        // ---------------- EDIT MENU ----------------
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Add Scene")) {
                Entity newScene = createSceneFlowNode();
                setStatusMessage(newScene != INVALID_ENTITY ? "Created new scene." : "Failed to create scene.");
            }

            if (ImGui::BeginMenu("Add Event")) {
                // Try to resolve current scene
                Entity targetNode = getPreferredSceneNode();
                const bool canAddDirect = (targetNode != INVALID_ENTITY);
                if (!canAddDirect) {
                    ImGui::TextDisabled("Select a FlowNode in Hierarchy. If none, a popup will appear.");
                }

                // Helper to handle click for a type
                auto handleAddEvent = [&](ComponentType type) {
                    if (canAddDirect) {
                        Entity e = createEventAndAttach(type, targetNode);
                        setStatusMessage(e != INVALID_ENTITY ? "Added event to scene." : "Failed to add event.");
                    } else {
                        s_pendingEventType = type;
                        s_openChooseScenePopup = true; // Open after menu bar
                    }
                };

                if (ImGui::MenuItem("Add Dialogue", nullptr, false)) {
                    handleAddEvent(ComponentType::Dialogue);
                }
                if (ImGui::MenuItem("Add Choice", nullptr, false)) {
                    handleAddEvent(ComponentType::Choice);
                }
                if (ImGui::MenuItem("Add Dice Roll", nullptr, false)) {
                    handleAddEvent(ComponentType::DiceRoll);
                }

                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Rename Selected File")) {
                showRenamePopup = true;
                strcpy(newName, m_selectedAssetName.c_str());
                ImGui::OpenPopup("RenameAssetPopup");
            }
            ImGui::EndMenu();
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
        // Keep using shared Play controls
        if (ImGui::BeginMenu("Run")) {
            bool playing = Editor_Run_IsPlaying();
            if (ImGui::MenuItem("Play", nullptr, false, !playing)) {
                Editor_Run_Play();
            }
            if (ImGui::MenuItem("Stop", nullptr, false, playing)) {
                Editor_Run_Stop();
            }
            if (ImGui::MenuItem("Restart", nullptr, false, playing)) {
                Editor_Run_Restart();
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    // ---------------- POPUPS ----------------

    // Open choose scene popup if requested from menu
    if (s_openChooseScenePopup) {
        ImGui::OpenPopup("Choose Scene for New Event");
        s_openChooseScenePopup = false;
    }

    if (ImGui::BeginPopupModal("Choose Scene for New Event", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        auto& em = EntityManager::get();
        Entity metaEntity = ProjectManager::getProjectMetaEntity();
        auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);

        ImGui::Text("Select a scene to attach the new event:");
        ImGui::Separator();

        if (!base) {
            ImGui::TextDisabled("No ProjectMeta found.");
        } else {
            auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);
            // List scenes as buttons
            for (Entity nodeId : meta->sceneNodes) {
                auto fn = em.getComponent<FlowNodeComponent>(nodeId);
                std::string label = fn ? fn->name : std::string("[Missing] ") + std::to_string((unsigned)nodeId);
                label += "##scene_pick_" + std::to_string((unsigned)nodeId);
                if (ImGui::Button(label.c_str())) {
                    Entity e = createEventAndAttach(s_pendingEventType, nodeId);
                    setStatusMessage(e != INVALID_ENTITY ? "Added event to selected scene." : "Failed to add event.");
                    ImGui::CloseCurrentPopup();
                }
            }
        }

        ImGui::Separator();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (showRenamePopup) {
        ImGui::OpenPopup("RenameAssetPopup");
        showRenamePopup = false;
    }

    if (showNewEntityPopup) {
        ImGui::OpenPopup("NewEntityPopup");
        showNewEntityPopup = false;
    }

    if (ProjectManager::consumeProjectInfoPrompt()) {
        openProjectInfoPopupOnce();
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
                        auto comp = make(); // fresh instance
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