#define NOMINMAX
#define IMGUI_DEFINE_MATH_OPERATORS
#include <glad/glad.h>
#include "EditorUI.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <vector>
#include <algorithm> 
#include <iostream>
#include <filesystem>
#include <fstream>
#include <random>
#include <json.hpp>
#include <unordered_map>

#include "UI/ImGuiUtils/ImGuiUtils.hpp"
#include "Resources/ResourceManager.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/ComponentRegistry.hpp"
#include "Engine/EntitySystem/Entity.hpp"
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Project/ProjectManager.hpp"
#include "Engine/RenderSystem/SceneManager.hpp"
// + Show event types in Hierarchy
#include "Engine/EntitySystem/Components/DialogueComponent.hpp"
#include "Engine/EntitySystem/Components/ChoiceComponent.hpp"
#include "Engine/EntitySystem/Components/DiceRollComponent.hpp"
#include "Engine/EntitySystem/Components/BackgroundComponent.hpp"
#include "Engine/EntitySystem/Components/ModelComponent.hpp"
#include "Engine/EntitySystem/Components/CharacterComponent.hpp"

#include "UI/FlowPanel/FlowCanvas.hpp"
#include "UI/FlowPanel/FlowEventsPanel.hpp"

void EditorUI::renderFlowTabs() {
    static std::string saveStatus;

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Flow Panel", nullptr, flags);
    if (ImGui::BeginTabBar("FlowTabs")) {
        if (ImGui::BeginTabItem("Flow")) {
            // Flowchart Canvas (drag/connect/persist)
            FlowCanvas::Render();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Events")) {
            // Scene events manager (list/reorder/add/remove/validate)
            FlowEventsPanel::Render();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Hierarchy")) {
            auto& em = EntityManager::get();
            Entity metaEntity = ProjectManager::getProjectMetaEntity();
            auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
            if (!base) {
                ImGui::TextDisabled("Project metadata missing.");
            } else {
                auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);

                // Removed "New Scene" button; use Edit -> Add Scene from menu instead.
                ImGui::TextDisabled("Right-click item to Set Start / Rename");

                ImGui::Separator();
                ImGui::Text("Scenes");
                ImGui::Separator();
                ImGui::BeginChild("HierarchyList", ImVec2(0, 0), true);

                // Rename popup state
                static bool openRename = false;
                static Entity renameId = INVALID_ENTITY;
                static char renameBuf[128] = {0};

                Entity currentSelected = getSelectedEntity();
                for (size_t i = 0; i < meta->sceneNodes.size(); ++i) {
                    Entity nodeId = meta->sceneNodes[i];
                    auto flowComp = em.getComponent(nodeId, ComponentType::FlowNode);
                    const char* missing = "[Missing FlowNode]";
                    std::string name;
                    if (flowComp) {
                        auto flow = std::static_pointer_cast<FlowNodeComponent>(flowComp);
                        name = flow->name;
                    } else {
                        name = missing;
                    }

                    bool isStart = (nodeId == meta->startNode);
                    bool isSceneSelected = (currentSelected == nodeId);

                    std::string header = name + " (ID: " + std::to_string(nodeId) + ")";
                    if (isStart) header += " [Start]";

                    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanFullWidth;
                    if (isSceneSelected) flags |= ImGuiTreeNodeFlags_Selected;

                    bool open = ImGui::TreeNodeEx((void*)(intptr_t)nodeId, flags, "%s", header.c_str());
                    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
                        setSelectedEntity(nodeId);
                    }

                    // Context menu per scene
                    if (ImGui::BeginPopupContextItem((std::string("ctx##") + std::to_string(nodeId)).c_str())) {
                        if (!isStart && ImGui::MenuItem("Set as Start")) {
                            // Unset previous start flag
                            if (meta->startNode != INVALID_ENTITY) {
                                if (auto prev = em.getComponent<FlowNodeComponent>(meta->startNode)) {
                                    prev->isStart = false;
                                }
                            }
                            // Set new start in both meta and component
                            meta->startNode = nodeId;
                            if (auto curr = em.getComponent<FlowNodeComponent>(nodeId)) {
                                curr->isStart = true;
                            }
                            ResourceManager::get().setUnsavedChanges(true);
                        }
                        if (ImGui::MenuItem("Rename")) {
                            renameId = nodeId;
                            if (auto flow = em.getComponent<FlowNodeComponent>(nodeId)) {
                                std::strncpy(renameBuf, flow->name.c_str(), sizeof(renameBuf));
                                renameBuf[sizeof(renameBuf) - 1] = '\0';
                            } else {
                                renameBuf[0] = '\0';
                            }
                            openRename = true;
                            ImGui::CloseCurrentPopup();
                        }
                        if (ImGui::MenuItem("Delete")) {
                            // Unlink references from other nodes (clear nextNode pointing to this)
                            for (Entity other : meta->sceneNodes) {
                                if (auto otherFn = em.getComponent<FlowNodeComponent>(other)) {
                                    if (otherFn->nextNode == static_cast<int>(nodeId)) {
                                        otherFn->nextNode = -1;
                                    }
                                }
                            }
                            bool wasStart = (meta->startNode == nodeId);
                            // Remove from sceneNodes
                            meta->sceneNodes.erase(std::remove(meta->sceneNodes.begin(), meta->sceneNodes.end(), nodeId),
                                                   meta->sceneNodes.end());
                            if (wasStart) {
                                meta->startNode = meta->sceneNodes.empty() ? INVALID_ENTITY : meta->sceneNodes.front();
                                if (meta->startNode != INVALID_ENTITY) {
                                    if (auto newStart = em.getComponent<FlowNodeComponent>(meta->startNode)) {
                                        newStart->isStart = true;
                                    }
                                }
                            }
                            if (getSelectedEntity() == nodeId) {
                                setSelectedEntity(INVALID_ENTITY);
                            }
                            ResourceManager::get().setUnsavedChanges(true);
                            setStatusMessage("Deleted scene from ProjectMeta (entity not destroyed).");
                        }
                        ImGui::EndPopup();
                    }

                    // Children: list contained events under the scene
                    if (open) {
                        auto fn = em.getComponent<FlowNodeComponent>(nodeId);
                        if (fn) {
                            // Events (existing)
                            for (int ei = 0; ei < (int)fn->eventSequence.size(); ++ei) {
                                Entity evt = fn->eventSequence[ei];
                                if (evt == INVALID_ENTITY) continue;

                                const char* typeLabel = "Unknown";
                                if (em.getComponent<DialogueComponent>(evt)) typeLabel = "Dialogue";
                                else if (em.getComponent<ChoiceComponent>(evt)) typeLabel = "Choice";
                                else if (em.getComponent<DiceRollComponent>(evt)) typeLabel = "Dice Roll";

                                bool isEvtSelected = (currentSelected == evt);
                                ImGuiTreeNodeFlags leafFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth;
                                if (isEvtSelected) leafFlags |= ImGuiTreeNodeFlags_Selected;

                                std::string childLabel = std::string(typeLabel) + " (ID: " + std::to_string((unsigned)evt) + ")";
                                ImGui::TreeNodeEx((void*)(intptr_t)evt, leafFlags, "%s", childLabel.c_str());
                                if (ImGui::IsItemClicked()) {
                                    setSelectedEntity(evt);
                                }
                            }

                            // Backgrounds
                            if (!fn->backgroundEntities.empty()) {
                                ImGui::Separator();
                                ImGui::TextDisabled("Background");
                                for (Entity bg : fn->backgroundEntities) {
                                    if (bg == INVALID_ENTITY) continue;
                                    bool isSel = (currentSelected == bg);
                                    ImGuiTreeNodeFlags leafFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth;
                                    if (isSel) leafFlags |= ImGuiTreeNodeFlags_Selected;
                                    std::string label = "Background (ID: " + std::to_string((unsigned)bg) + ")";
                                    ImGui::TreeNodeEx((void*)(intptr_t)bg, leafFlags, "%s", label.c_str());
                                    if (ImGui::IsItemClicked()) setSelectedEntity(bg);
                                }
                            }

                            // Objects
                            if (!fn->objectLayer.empty()) {
                                ImGui::Separator();
                                ImGui::TextDisabled("Objects");
                                for (Entity ob : fn->objectLayer) {
                                    if (ob == INVALID_ENTITY) continue;
                                    bool isSel = (currentSelected == ob);
                                    ImGuiTreeNodeFlags leafFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth;
                                    if (isSel) leafFlags |= ImGuiTreeNodeFlags_Selected;
                                    std::string label = "Object (ID: " + std::to_string((unsigned)ob) + ")";
                                    ImGui::TreeNodeEx((void*)(intptr_t)ob, leafFlags, "%s", label.c_str());
                                    if (ImGui::IsItemClicked()) setSelectedEntity(ob);
                                }
                            }

                            // Characters
                            if (!fn->characters.empty()) {
                                ImGui::Separator();
                                ImGui::TextDisabled("Characters");
                                for (Entity ch : fn->characters) {
                                    if (ch == INVALID_ENTITY) continue;
                                    bool isSel = (currentSelected == ch);
                                    ImGuiTreeNodeFlags leafFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth;
                                    if (isSel) leafFlags |= ImGuiTreeNodeFlags_Selected;
                                    std::string label = "Character (ID: " + std::to_string((unsigned)ch) + ")";
                                    ImGui::TreeNodeEx((void*)(intptr_t)ch, leafFlags, "%s", label.c_str());
                                    if (ImGui::IsItemClicked()) setSelectedEntity(ch);
                                }
                            }
                        }
                        ImGui::TreePop();
                    }
                }

                // Rename popup
                if (openRename) {
                    ImGui::OpenPopup("Rename Scene");
                    openRename = false;
                }
                if (ImGui::BeginPopupModal("Rename Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::InputText("Name", renameBuf, IM_ARRAYSIZE(renameBuf));
                    if (ImGui::Button("OK")) {
                        if (renameId != INVALID_ENTITY) {
                            if (auto flow = em.getComponent<FlowNodeComponent>(renameId)) {
                                flow->name = renameBuf;
                                setStatusMessage("Renamed scene.");
                                ResourceManager::get().setUnsavedChanges(true);
                            }
                        }
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel")) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                // Removed duplicate flat "Scene Contents" list; tree above shows events per scene.
                ImGui::EndChild();
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void EditorUI::renderSceneTabs() {
    // Call ScenePanel rendering (HUD overlay handled inside ScenePanel)
    scenePanel.renderScenePanel();
}

void EditorUI::renderInspectorTabs() {
    if (ImGui::Begin("Inspector")) {
        Entity selected = getSelectedEntity();
        if (selected == INVALID_ENTITY) {
            ImGui::TextDisabled("No entity selected.");
            ImGui::End();
            return;
        }

        // Centralized inspectors: handled inside EntityInspectorPanel.cpp
        renderEntityInspector(selected);

        // ...existing code...
    }
    ImGui::End();
}

void EditorUI::renderAssetBrowser() {
    ImGui::Begin("Assets Panel");
    ImVec2 panelSize = ImGui::GetContentRegionAvail();
    float leftPaneWidth = panelSize.x * 0.3f;

    ImGui::BeginChild("AssetFolders", ImVec2(leftPaneWidth, 0), true);
    renderFolderTree(m_assetsRoot, m_assetsRoot);
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("AssetPreview", ImVec2(0, 0), true);
    renderFolderPreview(m_selectedFolder);
    ImGui::EndChild();

    ImGui::End();
}