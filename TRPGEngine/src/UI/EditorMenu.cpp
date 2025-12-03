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
#include "UI/MenuHelpers/EditorMenuHelpers.hpp"
// Components needed for attach routing
#include "Engine/EntitySystem/Components/UIButtonComponent.hpp"

// Helpers for scene/event creation from the Edit menu
namespace {
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
				Entity newScene = EditorMenuHelpers::createSceneFlowNode();
				setStatusMessage(newScene != INVALID_ENTITY ? "Created new scene." : "Failed to create scene.");
			}
			if (ImGui::BeginMenu("Add Event")) {
				// Try to resolve current scene
				Entity targetNode = EditorMenuHelpers::getPreferredSceneNode();
				const bool canAddDirect = (targetNode != INVALID_ENTITY);
				if (!canAddDirect) {
					ImGui::TextDisabled("Select a FlowNode in Hierarchy. If none, a popup will appear.");
				}

				auto handleAddEvent = [&](ComponentType type) {
					if (canAddDirect) {
						Entity e = EditorMenuHelpers::createEventAndAttach(type, targetNode);
						setStatusMessage(e != INVALID_ENTITY ? "Added event to scene." : "Failed to add event.");
					} else {
						s_pendingEventType = type;
						s_openChooseScenePopup = true; // Unified chooser
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

			if (ImGui::MenuItem("Add Entity")) {
				showNewEntityPopup = true; // New Entity popup includes scene selection
			}

			if (ImGui::MenuItem("Rename Selected File")) {
				showRenamePopup = true;
				strcpy(newName, m_selectedAssetName.c_str());
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
	if (s_openChooseScenePopup) {
		ImGui::OpenPopup("Choose Scene");
		s_openChooseScenePopup = false;
	}

	if (ImGui::BeginPopupModal("Choose Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		auto& em = EntityManager::get();
		Entity metaEntity = ProjectManager::getProjectMetaEntity();
		auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);

		ImGui::TextUnformatted("Select a scene to attach:");
		ImGui::Separator();

		if (!base) {
			ImGui::TextDisabled("No ProjectMeta found.");
		} else {
			auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);
			for (Entity nodeId : meta->sceneNodes) {
				auto fn = em.getComponent<FlowNodeComponent>(nodeId);
				std::string label = fn ? fn->name : std::string("[Missing] ") + std::to_string((unsigned)nodeId);
				label += "##scene_pick_" + std::to_string((unsigned)nodeId);
				if (ImGui::Button(label.c_str())) {
					Entity e = EditorMenuHelpers::createEventAndAttach(s_pendingEventType, nodeId);
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

	// Build scene list and default selection (prefer current FlowNode or first scene)
	static Entity selectedSceneForNewEntity = INVALID_ENTITY;
	std::vector<std::pair<Entity, std::string>> sceneItems;
	{
		auto& em = EntityManager::get();
		Entity metaEntity = ProjectManager::getProjectMetaEntity();
		auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
		if (base) {
			auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);
			for (Entity nodeId : meta->sceneNodes) {
				auto fn = em.getComponent<FlowNodeComponent>(nodeId);
				std::string label = fn ? fn->name : std::string("[Missing] ") + std::to_string((unsigned)nodeId);
				sceneItems.emplace_back(nodeId, label);
			}
		}
		// Default selection if needed
		if (selectedSceneForNewEntity == INVALID_ENTITY) {
			// Prefer current FlowNode
			Entity cur = SceneManager::get().getCurrentFlowNode();
			if (cur != INVALID_ENTITY) selectedSceneForNewEntity = cur;
			else if (!sceneItems.empty()) selectedSceneForNewEntity = sceneItems.front().first;
		}
	}

	if (ImGui::BeginPopupModal("NewEntityPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		shouldClosePopup = false;

		// Scene selector inside New Entity popup
		ImGui::TextUnformatted("Attach to Scene:");
		if (sceneItems.empty()) {
			ImGui::TextDisabled("No scenes found. Create a Scene first.");
		} else {
			// Find current label
			std::string currentLabel = "[None]";
			for (auto& it : sceneItems) {
				if (it.first == selectedSceneForNewEntity) { currentLabel = it.second; break; }
			}
			if (ImGui::BeginCombo("##AttachSceneCombo", currentLabel.c_str())) {
				for (auto& it : sceneItems) {
					bool sel = (it.first == selectedSceneForNewEntity);
					if (ImGui::Selectable(it.second.c_str(), sel)) {
						selectedSceneForNewEntity = it.first;
					}
					if (sel) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
		ImGui::Separator();

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

				// Track if we handled creation as an event (to skip entity branch)
				bool createdEvent = false;

				// If user picked an Event type, delegate to event creator and attach to selected scene
				if (type == EntityType::Dialogue || type == EntityType::Choice || type == EntityType::DiceRoll) {
					ComponentType ctype = ComponentType::Unknown;
					if (type == EntityType::Dialogue) ctype = ComponentType::Dialogue;
					else if (type == EntityType::Choice) ctype = ComponentType::Choice;
					else if (type == EntityType::DiceRoll) ctype = ComponentType::DiceRoll;
					Entity evt = (selectedSceneForNewEntity != INVALID_ENTITY)
						? EditorMenuHelpers::createEventAndAttach(ctype, selectedSceneForNewEntity)
						: INVALID_ENTITY;
					setStatusMessage(evt != INVALID_ENTITY ? "Added event to scene." : "Failed to add event.");
					if (evt != INVALID_ENTITY) {
						createdEvent = true;
						shouldClosePopup = true;       // close properly later
						nameBuffer[0] = '\0';          // reset for next time
					}
				}
				// If not created as an event, proceed with entity creation (Background/Character/UIButton/etc)
				if (!createdEvent) {
					Entity e = EntityManager::get().createEntity(EntityManager::get().getSelectedEntity());
					std::cout << "Created new entity with ID: " << e << std::endl;

					EntityManager::get().setEntityMeta(e, nameBuffer, type);
					std::cout << "Entity metadata set. Name: " << nameBuffer << ", Type: " << static_cast<int>(type) << std::endl;

					auto it = templateMap.find(type);
					if (it != templateMap.end()) {
						size_t added = 0;
						for (const auto& make : it->second.factories) {
							if (!make) { std::cout << "[ERROR] Null factory in template!\n"; continue; }
							auto comp = make();
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

						// Attach by type to selectedSceneForNewEntity
						auto& em = EntityManager::get();
						if (auto bgComp = em.getComponent<BackgroundComponent>(e)) {
							// Transform2D defaults removed to avoid redefinition errors
						}

						if (selectedSceneForNewEntity != INVALID_ENTITY) {
							if (auto fn = em.getComponent<FlowNodeComponent>(selectedSceneForNewEntity)) {
								if (em.getComponent<BackgroundComponent>(e)) {
									fn->backgroundEntities.clear();
									fn->backgroundEntities.push_back(e);
								} else if (em.getComponent<CharacterComponent>(e)) {
									fn->characters.push_back(e);
								} else if (em.getComponent<UIButtonComponent>(e)) {
									fn->uiLayer.push_back(e);
								}
								SceneManager::get().setCurrentFlowNode(selectedSceneForNewEntity);
							}
						}
						ResourceManager::get().setUnsavedChanges(true);

						// Reset chooser state and close
						shouldClosePopup = true;
						nameBuffer[0] = '\0';
					}
				} // end if !createdEvent
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