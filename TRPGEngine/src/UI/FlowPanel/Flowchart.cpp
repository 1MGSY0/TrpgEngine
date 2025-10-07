#include "Flowchart.hpp"
#include "Engine/RenderSystem/SceneManager.hpp"
#include "Project/ProjectManager.hpp" 
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/EntitySystem/EntityManager.tpp"
#include <imgui_internal.h>
#include <cmath>
#include <iostream> 

void Flowchart::addNode(const std::string& id, const std::string& label, const ImVec2& pos) {
    if (m_nodes.find(id) == m_nodes.end()) {
        m_nodes[id] = FlowNode{id, label, pos};
    }
}

void Flowchart::addConnection(const std::string& from, const std::string& to) {
    m_connections.push_back({from, to});
}

void Flowchart::render() {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();

    const float nodeW = 140.0f;
    const float nodeH = 44.0f;
    const float vGap  = 90.0f;   // vertical spacing
    const float hGap  = 220.0f;  // space for edges

    auto& em = EntityManager::get();

    // 1) Grab meta (ensure it exists somewhere else in your init)
    Entity metaE = ProjectManager::getProjectMetaEntity();
    if (metaE == INVALID_ENTITY) {
        std::cout << "[Flowchart] meta entity INVALID\n";
        return;
    }
    auto metaBase = em.getComponent(metaE, ComponentType::ProjectMetadata);
    if (!metaBase) {
        std::cout << "[Flowchart] ProjectMetaComponent missing on " << (int)metaE << "\n";
        return;
    }
    auto meta = std::static_pointer_cast<ProjectMetaComponent>(metaBase);

    // 2) Sync: rebuild sceneNodes from actual ECS every frame
    {
        auto nodes = em.getEntitiesWith(ComponentType::FlowNode);
        std::sort(nodes.begin(), nodes.end());
        // Only replace if different to avoid unnecessary churn
        if (nodes != meta->sceneNodes) {
            meta->sceneNodes = nodes;
            std::cout << "[Flowchart] sync sceneNodes from ECS -> " << meta->sceneNodes.size() << " nodes\n";
        }

        // If we just discovered our first node, make it the start/current
        if (!meta->sceneNodes.empty() && meta->startNode == INVALID_ENTITY) {
            meta->startNode = meta->sceneNodes.front();
            SceneManager::get().setCurrentFlowNode(meta->startNode);
            std::cout << "[Flowchart] auto-set startNode/current = " << (int)meta->startNode << "\n";
        }
    }

    // Ensure that any nextNode targets that exist are also present in sceneNodes.
    // If a target doesn’t exist anymore → sanitize it to INVALID_ENTITY.
    {
        bool changed = false;

        // Fast membership test for existing list
        std::unordered_set<Entity> present(meta->sceneNodes.begin(), meta->sceneNodes.end());

        for (Entity src : meta->sceneNodes) {
            auto flow = em.getComponent<FlowNodeComponent>(src);
            if (!flow) continue;

            const Entity dst = flow->nextNode;
            if (dst == INVALID_ENTITY) continue;

            // If target entity no longer exists or has no FlowNodeComponent → clear the link
            if (!em.entityExists(dst) || !em.getComponent<FlowNodeComponent>(dst)) {
                std::cout << "[Flowchart] sanitize: " << (int)src << " -> (dead) " << (int)dst << " -> INVALID\n";
                flow->nextNode = INVALID_ENTITY;
                changed = true;
                continue;
            }

            // If it exists but isn’t listed yet, include it so the edge can render
            if (!present.count(dst)) {
                meta->sceneNodes.push_back(dst);
                present.insert(dst);
                changed = true;
                std::cout << "[Flowchart] include target: " << (int)dst << " (was referenced by " << (int)src << ")\n";
            }
        }

        if (changed) {
            // Keep order stable
            std::sort(meta->sceneNodes.begin(), meta->sceneNodes.end());
            meta->sceneNodes.erase(std::unique(meta->sceneNodes.begin(), meta->sceneNodes.end()), meta->sceneNodes.end());
        }
    }



    // Debug line (now shows live numbers)
    std::cout << "[Flowchart] rendering; sceneNodes=" << meta->sceneNodes.size()
              << " startNode=" << (int)meta->startNode
              << " current="   << (int)SceneManager::get().getCurrentFlowNode() << "\n";

    // 3) Build visuals
    if (m_nodes.empty()) m_nodes.reserve(meta->sceneNodes.size());
    // Drop visuals for nodes that no longer exist
    for (auto it = m_nodes.begin(); it != m_nodes.end(); ) {
        Entity id = (Entity)std::stoul(it->first);
        if (std::find(meta->sceneNodes.begin(), meta->sceneNodes.end(), id) == meta->sceneNodes.end())
            it = m_nodes.erase(it);
        else ++it;
    }
    // Add/initialize visuals for existing nodes
    for (size_t i = 0; i < meta->sceneNodes.size(); ++i) {
        Entity id = meta->sceneNodes[i];
        auto flow = em.getComponent<FlowNodeComponent>(id);
        if (!flow) continue;
        std::string key = std::to_string(id);
        if (!m_nodes.count(key)) {
            // simple vertical list layout
            ImVec2 pos(origin.x + 40.0f, origin.y + i * vGap);
            addNode(key, flow->name.empty() ? ("Node " + key) : flow->name, pos);
        } else {
            // keep label up to date
            m_nodes[key].label = flow->name.empty() ? ("Node " + key) : flow->name;
        }
    }

    // 4) Render nodes + interactions
    Entity current = SceneManager::get().getCurrentFlowNode();
    for (auto& [key, node] : m_nodes) {
        Entity id = (Entity)std::stoul(key);

        ImU32 colFill   = IM_COL32(80, 120, 255, 255);
        ImU32 colBorder = IM_COL32(10, 10, 10, 255);
        if (id == meta->startNode) colFill = IM_COL32(60, 180, 90, 255);   // start = green
        if (id == current)         colFill = IM_COL32(220, 120, 60, 255);  // current = orange

        ImVec2 p0 = node.position;
        ImVec2 p1 = ImVec2(node.position.x + nodeW, node.position.y + nodeH);

        dl->AddRectFilled(p0, p1, colFill, 4.0f);
        dl->AddRect(p0, p1, colBorder, 4.0f, 0, 2.0f);
        dl->AddText(ImVec2(p0.x + 8, p0.y + 10), IM_COL32(255,255,255,255), node.label.c_str());

        // Dragging
        ImGui::SetCursorScreenPos(p0);
        ImGui::PushID(key.c_str());
        ImGui::InvisibleButton("node", ImVec2(nodeW, nodeH));
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            ImVec2 d = ImGui::GetIO().MouseDelta;
            node.position.x += d.x;
            node.position.y += d.y;
        }

        // Double-click to jump current node
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            SceneManager::get().setCurrentFlowNode(id);
            std::cout << "[Flowchart] jump to node " << (int)id << "\n";
        }

        // Context menu: set start / connect to next
        if (ImGui::BeginPopupContextItem("FlowNodeCtx")) {
            if (ImGui::MenuItem("Set as Start Node")) {
                meta->startNode = id;
                std::cout << "[Flowchart] set startNode = " << (int)id << "\n";
            }
            auto flow = em.getComponent<FlowNodeComponent>(id);
            if (flow) {
                if (ImGui::BeginMenu("Connect to...")) {
                    for (Entity target : meta->sceneNodes) {
                        if (target == id) continue;
                        std::string lab = std::to_string(target);
                        auto tflow = em.getComponent<FlowNodeComponent>(target);
                        if (tflow && !tflow->name.empty()) lab += " - " + tflow->name;
                        bool sel = (flow->nextNode == target);
                        if (ImGui::MenuItem(lab.c_str(), nullptr, sel)) {
                            flow->nextNode = target;
                            std::cout << "[Flowchart] " << (int)id << " -> next = " << (int)target << "\n";
                        }
                    }
                    ImGui::EndMenu();
                }
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    // 5) Render connections (edges) based on FlowNodeComponent::nextNode
    for (Entity id : meta->sceneNodes) {
        auto flow = em.getComponent<FlowNodeComponent>(id);
        if (!flow || flow->nextNode == INVALID_ENTITY) continue;
        std::string fromK = std::to_string(id);
        std::string toK   = std::to_string(flow->nextNode);
        if (!m_nodes.count(fromK) || !m_nodes.count(toK)) {
            std::cout << "[Flowchart] skip edge " << fromK << " -> " << toK << " (missing node)\n";
            continue;
        }
        ImVec2 A = ImVec2(m_nodes[fromK].position.x + nodeW, m_nodes[fromK].position.y + nodeH*0.5f);
        ImVec2 B = ImVec2(m_nodes[toK].position.x,           m_nodes[toK].position.y   + nodeH*0.5f);
        dl->AddBezierCubic(A, ImVec2(A.x + 40, A.y), ImVec2(B.x - 40, B.y), B,
                           IM_COL32(200,200,100,255), 2.0f);

        // arrow
        ImVec2 d = ImVec2(B.x - A.x, B.y - A.y);
        float len = std::sqrt(d.x*d.x + d.y*d.y);
        if (len > 0.01f) {
            ImVec2 n = ImVec2(d.x/len, d.y/len);
            ImVec2 left = ImVec2(-n.y*5.0f, n.x*5.0f);
            ImVec2 tip = B;
            ImVec2 base = ImVec2(tip.x - n.x*12.0f, tip.y - n.y*12.0f);
            dl->AddTriangleFilled(tip,
                                  ImVec2(base.x + left.x, base.y + left.y),
                                  ImVec2(base.x - left.x, base.y - left.y),
                                  IM_COL32(200,200,100,255));
        }
    }
}