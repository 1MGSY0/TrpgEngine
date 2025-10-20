#define NOMINMAX
#define IMGUI_DEFINE_MATH_OPERATORS
#include <glad/glad.h>
#include "EditorUI.hpp"

#include <imgui.h>
#include <vector>
#include <algorithm> 
#include <iostream>
#include <filesystem>
#include <json.hpp> // for nlohmann::json
#include <unordered_map> // + flowchart layout

#include "UI/ImGuiUtils/ImGuiUtils.hpp"
#include "Resources/ResourceManager.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/ComponentRegistry.hpp"
#include "Engine/EntitySystem/Entity.hpp"
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Project/ProjectManager.hpp"  // + ensure meta access

#include "Engine/RenderSystem/SceneManager.hpp"

void EditorUI::renderFlowTabs() {
    static std::string saveStatus;

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Flow Panel", nullptr, flags);
    if (ImGui::BeginTabBar("FlowTabs")) {
        if (ImGui::BeginTabItem("Flow")) {
            // Minimal built-in canvas with drag-to-connect behavior
            auto& em = EntityManager::get();
            Entity metaEntity = ProjectManager::getProjectMetaEntity();
            auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
            if (base) {
                auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);

                ImGui::Separator();
                ImGui::TextDisabled("Flowchart (MVP): drag nodes; drag from port to connect; right-click to clear link");
                ImGui::BeginChild("FlowchartCanvas", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
                ImDrawList* draw = ImGui::GetWindowDrawList();
                ImVec2 origin = ImGui::GetCursorScreenPos();
                ImVec2 canvasSize = ImGui::GetContentRegionAvail();

                // State
                static std::unordered_map<Entity, ImVec2> s_nodePos;
                static Entity s_draggingNode = INVALID_ENTITY;
                static ImVec2 s_dragOffset = ImVec2(0, 0);
                static Entity s_linkFrom = INVALID_ENTITY;

                // Defaults for new nodes
                auto ensurePos = [&](Entity e, int idx) {
                    if (s_nodePos.find(e) == s_nodePos.end()) {
                        float x = 40.0f + (idx % 5) * 200.0f;
                        float y = 40.0f + (idx / 5) * 140.0f;
                        s_nodePos[e] = ImVec2(x, y);
                    }
                };

                // Draw links first (under nodes)
                for (size_t i = 0; i < meta->sceneNodes.size(); ++i) {
                    Entity nodeId = meta->sceneNodes[i];
                    ensurePos(nodeId, (int)i);
                    auto fn = em.getComponent<FlowNodeComponent>(nodeId);
                    if (!fn) continue;

                    if (fn->nextNode >= 0) {
                        Entity to = (Entity)fn->nextNode;
                        if (s_nodePos.find(to) == s_nodePos.end()) continue;
                        ImVec2 fromPos = origin + s_nodePos[nodeId] + ImVec2(160, 30);
                        ImVec2 toPos = origin + s_nodePos[to] + ImVec2(0, 30);
                        ImU32 col = IM_COL32(140, 200, 255, 255);
                        draw->AddBezierCubic(fromPos, fromPos + ImVec2(50, 0), toPos - ImVec2(50, 0), toPos, col, 2.0f);
                    }
                }

                // Node rendering and interaction
                Entity hoveredNode = INVALID_ENTITY;
                for (size_t i = 0; i < meta->sceneNodes.size(); ++i) {
                    Entity nodeId = meta->sceneNodes[i];
                    ensurePos(nodeId, (int)i);
                    auto fn = em.getComponent<FlowNodeComponent>(nodeId);
                    std::string title = fn ? fn->name : ("[Missing] " + std::to_string(nodeId));

                    ImVec2 npos = origin + s_nodePos[nodeId];
                    ImVec2 size(160, 60);
                    ImVec2 min = npos;
                    ImVec2 max = npos + size;

                    bool isStart = (meta->startNode == nodeId);
                    ImU32 bgCol = isStart ? IM_COL32(60, 140, 80, 255) : IM_COL32(50, 50, 60, 255);
                    ImU32 borderCol = IM_COL32(90, 90, 110, 255);
                    draw->AddRectFilled(min, max, bgCol, 6.0f);
                    draw->AddRect(min, max, borderCol, 6.0f);

                    // Title
                    draw->AddText(min + ImVec2(8, 8), IM_COL32_WHITE, title.c_str());

                    // Input/Output ports
                    ImVec2 inPort = min + ImVec2(-6, 30);
                    ImVec2 outPort = max + ImVec2(6, -30);
                    draw->AddCircleFilled(inPort, 6.0f, IM_COL32(200, 200, 200, 255));
                    draw->AddCircleFilled(outPort, 6.0f, IM_COL32(200, 200, 200, 255));

                    // Hover detection and select
                    bool hovered = ImGui::IsMouseHoveringRect(min, max);
                    if (hovered) hoveredNode = nodeId;

                    // Drag node (LMB on node body)
                    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                        s_draggingNode = nodeId;
                        s_dragOffset = ImGui::GetMousePos() - npos;
                        setSelectedEntity(nodeId);
                        SceneManager::get().setCurrentFlowNode(nodeId);
                    }
                    if (s_draggingNode == nodeId && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                        s_nodePos[nodeId] = ImGui::GetMousePos() - origin - s_dragOffset;
                    }
                    if (s_draggingNode == nodeId && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                        s_draggingNode = INVALID_ENTITY;
                    }

                    // Start linking when clicking output port
                    if (ImGui::IsMouseHoveringRect(outPort - ImVec2(8, 8), outPort + ImVec2(8, 8))) {
                        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                            s_linkFrom = nodeId;
                        }
                    }
                    // Clear link on right-click output port
                    if (ImGui::IsMouseHoveringRect(outPort - ImVec2(8, 8), outPort + ImVec2(8, 8))) {
                        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                            if (auto src = em.getComponent<FlowNodeComponent>(nodeId)) {
                                src->nextNode = -1;
                                ResourceManager::get().setUnsavedChanges(true);
                            }
                        }
                    }

                    // Draw live linking line
                    if (s_linkFrom != INVALID_ENTITY) {
                        ImVec2 fromPos = origin + s_nodePos[s_linkFrom] + ImVec2(160, 30);
                        ImVec2 mousePos = ImGui::GetMousePos();
                        draw->AddBezierCubic(fromPos, fromPos + ImVec2(50, 0), mousePos - ImVec2(50, 0), mousePos, IM_COL32(255, 255, 100, 255), 2.0f);
                    }

                    // Accept link drop on input port
                    if (ImGui::IsMouseHoveringRect(inPort - ImVec2(8, 8), inPort + ImVec2(8, 8))) {
                        if (s_linkFrom != INVALID_ENTITY && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                            if (auto src = em.getComponent<FlowNodeComponent>(s_linkFrom)) {
                                src->nextNode = (int)nodeId;
                                ResourceManager::get().setUnsavedChanges(true);
                            }
                            s_linkFrom = INVALID_ENTITY;
                        }
                    }
                }

                // Cancel linking if released elsewhere
                if (s_linkFrom != INVALID_ENTITY && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                    s_linkFrom = INVALID_ENTITY;
                }

                ImGui::EndChild();
            }

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Events")) {
            ImGui::Text("Event list...");
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

                if (ImGui::Button("New Scene")) {
                    Entity e = em.createEntity(INVALID_ENTITY);

                    const auto* info = ComponentTypeRegistry::getInfo(ComponentType::FlowNode);
                    if (info && info->loader) {
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
                        if (comp && em.addComponent(e, comp) == EntityManager::AddComponentResult::Ok) {
                            comp->Init(e);

                            auto fn = em.getComponent<FlowNodeComponent>(e);
                            if (fn) {
                                if (fn->name.empty()) fn->name = defName;
                            }

                            meta->sceneNodes.push_back(e);

                            // If first scene, set as start in both meta and component
                            if (meta->startNode == INVALID_ENTITY) {
                                meta->startNode = e;
                                if (fn) fn->isStart = true;
                            }

                            // If some node is selected and has no next, auto-link it to this new node
                            Entity selected = getSelectedEntity();
                            if (selected != INVALID_ENTITY) {
                                if (auto selFn = em.getComponent<FlowNodeComponent>(selected)) {
                                    if (selFn->nextNode < 0) {
                                        selFn->nextNode = static_cast<int>(e);
                                    }
                                }
                            }

                            setSelectedEntity(e);
                            SceneManager::get().setCurrentFlowNode(e);
                            ResourceManager::get().setUnsavedChanges(true);

                            setStatusMessage("Created new scene: " + defName);
                        } else {
                            setStatusMessage("Failed to add FlowNode component.");
                        }
                    } else {
                        setStatusMessage("FlowNode factory not found.");
                    }
                }

                ImGui::SameLine();
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
                    bool isSelected = (currentSelected == nodeId);

                    std::string label = name + " (ID: " + std::to_string(nodeId) + ")";
                    if (isStart) label += " [Start]";

                    if (ImGui::Selectable(label.c_str(), isSelected)) {
                        setSelectedEntity(nodeId);
                    }

                    // Context menu per item
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
                            // Preload current name
                            if (auto flow = em.getComponent<FlowNodeComponent>(nodeId)) {
                                std::strncpy(renameBuf, flow->name.c_str(), sizeof(renameBuf));
                                renameBuf[sizeof(renameBuf) - 1] = '\0';
                            } else {
                                renameBuf[0] = '\0';
                            }
                            openRename = true;
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
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

                ImGui::EndChild();
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void EditorUI::renderSceneTabs() {
    //Call ScenePanel rendering
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

        // Removed duplicate tag list and add/remove component controls.
        // These are now managed inside renderEntityInspector(...) for consistency.
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