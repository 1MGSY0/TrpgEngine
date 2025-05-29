#include "Flowchart.hpp"
#include "Engine/RenderSystem/SceneManager.hpp"
#include "core/EngineManager.hpp"
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include <imgui_internal.h>
#include <cmath>

void Flowchart::addNode(const std::string& id, const std::string& label, const ImVec2& pos) {
    if (m_nodes.find(id) == m_nodes.end()) {
        m_nodes[id] = FlowNode{id, label, pos};
    }
}

void Flowchart::addConnection(const std::string& from, const std::string& to) {
    m_connections.push_back({from, to});
}

void Flowchart::render() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();

    const float nodeWidth = 120.0f;
    const float nodeHeight = 40.0f;
    const float spacing = 150.0f;

    auto& em = EntityManager::get();
    Entity metaEntity = EngineManager::get().getProjectMetaEntity();
    auto metaBase = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
    if (!metaBase) return;

    auto meta = std::static_pointer_cast<ProjectMetaComponent>(metaBase);

    // Add all FlowNodes as visual nodes
    for (size_t i = 0; i < meta->sceneNodes.size(); ++i) {
        Entity nodeId = meta->sceneNodes[i];
        auto flowComp = em.getComponent<FlowNodeComponent>(nodeId);
        if (!flowComp) continue;

        std::string label = flowComp->name;
        std::string id = std::to_string(nodeId);

        if (m_nodes.find(id) == m_nodes.end()) {
            ImVec2 pos = ImVec2(origin.x + 100.0f, origin.y + i * spacing);
            addNode(id, label, pos);
        }
    }

    // Render visual node widgets
    for (auto& [id, node] : m_nodes) {
        ImGui::SetCursorScreenPos(node.position);
        ImGui::PushID(id.c_str());

        ImVec2 rectMax(node.position.x + nodeWidth, node.position.y + nodeHeight);
        drawList->AddRectFilled(node.position, rectMax, IM_COL32(80, 120, 255, 255), 4.0f);
        drawList->AddText(ImVec2(node.position.x + 10, node.position.y + 10), IM_COL32(255, 255, 255, 255), node.label.c_str());

        ImGui::InvisibleButton("node_drag", ImVec2(nodeWidth, nodeHeight));
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            node.position.x += ImGui::GetIO().MouseDelta.x;
            node.position.y += ImGui::GetIO().MouseDelta.y;
        }

        ImGui::PopID();
    }

    // Add connections based on nextNode in FlowNodeComponent
    m_connections.clear();
    for (Entity nodeId : meta->sceneNodes) {
        auto flowComp = em.getComponent<FlowNodeComponent>(nodeId);
        if (!flowComp || flowComp->nextNode == INVALID_ENTITY) continue;

        std::string from = std::to_string(nodeId);
        std::string to   = std::to_string(flowComp->nextNode);
        if (m_nodes.count(from) && m_nodes.count(to)) {
            addConnection(from, to);
        }
    }

    renderConnections();
}

void Flowchart::renderConnections() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    for (const auto& conn : m_connections) {
        if (!m_nodes.count(conn.fromId) || !m_nodes.count(conn.toId)) continue;

        const auto& from = m_nodes[conn.fromId];
        const auto& to   = m_nodes[conn.toId];

        ImVec2 p1 = ImVec2(from.position.x + 120, from.position.y + 20);
        ImVec2 p2 = ImVec2(to.position.x, to.position.y + 20);
        drawArrow(drawList, p1, p2);
    }
}

void Flowchart::drawArrow(ImDrawList* drawList, const ImVec2& start, const ImVec2& end) {
    drawList->AddBezierCubic(
        start,
        ImVec2(start.x + 50, start.y),
        ImVec2(end.x - 50, end.y),
        end,
        IM_COL32(200, 200, 100, 255),
        2.0f
    );

    ImVec2 dir = ImVec2(end.x - start.x, end.y - start.y);
    float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.1f) {
        ImVec2 norm = ImVec2(dir.x / len, dir.y / len);
        ImVec2 left = ImVec2(-norm.y * 5.0f, norm.x * 5.0f);
        ImVec2 tip = end;
        ImVec2 base = ImVec2(tip.x - norm.x * 10.0f, tip.y - norm.y * 10.0f);

        drawList->AddTriangleFilled(
            tip,
            ImVec2(base.x + left.x, base.y + left.y),
            ImVec2(base.x - left.x, base.y - left.y),
            IM_COL32(200, 200, 100, 255)
        );
    }
}