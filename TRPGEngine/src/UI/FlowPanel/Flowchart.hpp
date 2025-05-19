#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <imgui.h>
#include <memory>

struct FlowNode {
    std::string id;
    std::string label;
    ImVec2 position;
    bool selected = false;
};

struct FlowConnection {
    std::string fromId;
    std::string toId;
};

class Flowchart {
public:
    std::string m_lastCreatedNodeId;

    void addNode(const std::string& id, const std::string& label, const ImVec2& pos);
    void addConnection(const std::string& from, const std::string& to);
    void render();  // Master render call
    void addSceneNodeWithAutoConnection(const std::string& sceneName);

private:
    void renderNodes();              // Draws nodes with drag & drop
    void renderConnections();        // Draws arrows between connected nodes
    void drawArrow(ImDrawList* drawList, const ImVec2& from, const ImVec2& to); // Bezier arrow + head
    void handleInteraction();        // For advanced interaction support (TBD)

private:
    std::unordered_map<std::string, FlowNode> m_nodes;
    std::vector<FlowConnection> m_connections;
};