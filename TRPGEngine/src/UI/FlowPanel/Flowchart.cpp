#include "Flowchart.hpp"
#include "UI/SceneManager.hpp"
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

    auto& scenes = SceneManager::getSceneNames();
    const float nodeWidth = 120.0f;
    const float nodeHeight = 40.0f;
    const float spacing = 150.0f;

    static int cachedNextIndex = -1;
    static std::string cachedPreviewSceneName;
    static std::string lastHoveredNodeId;

    for (size_t i = 0; i < scenes.size(); ++i) {
        const std::string& name = scenes[i];
        if (m_nodes.find(name) == m_nodes.end()) {
            ImVec2 defaultPos = ImVec2(origin.x + 100.0f, origin.y + 100.0f + i * spacing);
            addNode(name, name, defaultPos);
        }
    }

    for (auto& [id, node] : m_nodes) {
        ImGui::SetCursorScreenPos(node.position);
        ImGui::PushID(id.c_str());

        ImVec2 rectMax(node.position.x + nodeWidth, node.position.y + nodeHeight);
        drawList->AddRectFilled(node.position, rectMax, IM_COL32(80, 120, 255, 255), 4.0f);
        drawList->AddText(ImVec2(node.position.x + 10, node.position.y + 10), IM_COL32(255, 255, 255, 255), node.label.c_str());

        ImGui::SetCursorScreenPos(node.position);
        ImGui::InvisibleButton("node_drag", ImVec2(nodeWidth, nodeHeight));
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            node.position.x += ImGui::GetIO().MouseDelta.x;
            node.position.y += ImGui::GetIO().MouseDelta.y;
        }

        // Position the + button
        ImVec2 plusPos = ImVec2(node.position.x + 45, node.position.y + 50);
        ImGui::SetCursorScreenPos(plusPos);

        bool isPlusClicked = ImGui::Button("+##AddScene");
        bool isPlusHovered = ImGui::IsItemHovered();

        if (isPlusClicked) {
            std::string newScene = "Scene " + std::to_string(SceneManager::getNextSceneIndex());
            SceneManager::addScene(newScene);

            // Position the new node below the current node
            ImVec2 newPos = ImVec2(node.position.x, node.position.y + spacing);
            addNode(newScene, newScene, newPos);

            addConnection(id, newScene);
        }

        if (isPlusHovered) {
            if (lastHoveredNodeId >= id) {
                // Cache only on first hover for this node
                cachedNextIndex = SceneManager::getNextSceneIndex();
                cachedPreviewSceneName = "Scene " + std::to_string(cachedNextIndex);
                lastHoveredNodeId = id;
            }

            // Draw preview node
            ImVec2 previewPos = ImVec2(plusPos.x, plusPos.y + 60);
            drawList->AddRectFilled(previewPos, ImVec2(previewPos.x + nodeWidth, previewPos.y + nodeHeight), IM_COL32(100, 200, 100, 255));
            drawList->AddText(ImVec2(previewPos.x + 10, previewPos.y + 10), IM_COL32(255, 255, 255, 255), cachedPreviewSceneName.c_str());

            ImVec2 startPoint = ImVec2(node.position.x + nodeWidth / 2, node.position.y + nodeHeight);
            ImVec2 endPoint = ImVec2(previewPos.x + nodeWidth / 2, previewPos.y);
            drawArrow(drawList, startPoint, endPoint);
        } else if (lastHoveredNodeId == id) {
            // Clear cache when no longer hovering this plus
            cachedNextIndex = -1;
            cachedPreviewSceneName.clear();
            lastHoveredNodeId.clear();
        }

        ImGui::PopID();
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