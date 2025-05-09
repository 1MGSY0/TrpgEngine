#include "EditorUI.h"
#include <vector>
#include <imgui.h>
#include <iostream>


#include "Engine/Assets/ImportManager.h"	
#include "Engine/Resources/ResourceManager.h"

#include "UI/ScenePanel/ScenePanel.h"
#include "UI/AssetPanels/TextPanel.h"
#include "UI/AssetPanels/CharacterPanel.h"
#include "UI/AssetPanels/AudioPanel.h"
#include "UI/IPanel.h"

void EditorUI::renderFlowTabs() {

    static std::string saveStatus;

    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    float height = displaySize.y * 0.65f;           // scale relative to screen height
    float width  = height * 0.415f;                 // maintain original ratio
    ImVec2 pos(0, displaySize.y * 0.10f);           // left side

    //ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    //ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Flow Panel", nullptr, flags);
    if (ImGui::BeginTabBar("FlowTabs")) {
        if (ImGui::BeginTabItem("Flow")) {
            ImGui::Text("Flow Graph content...");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Events")) {
            ImGui::Text("Event list...");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void EditorUI::renderSceneTabs() {

    static std::string saveStatus;
    static std::vector<std::string> sceneTabs = { "Scene 1" };  // initial scene
    static int nextSceneIndex = 2; // for naming new scenes

    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    float targetAspect = 867.0f / 628.0f;
    float height = displaySize.y * 0.6f;
    float width  = height * targetAspect;

    ImVec2 panelSize(width, height);
    ImVec2 panelPos(displaySize.x * 0.3f, displaySize.y * 0.15f); // center-ish

    //ImGui::SetNextWindowPos(panelPos, ImGuiCond_Always);
    //ImGui::SetNextWindowSize(panelSize, ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Scene Panel", nullptr, flags);
    if (ImGui::BeginTabBar("SceneTabs")) {
        // Render each tab
        for (int i = 0; i < sceneTabs.size(); ++i) {
            if (ImGui::BeginTabItem(sceneTabs[i].c_str())) {
                renderScenePanel();  // your function
                ImGui::EndTabItem();
            }
        }

        // Add "+" button styled as a tab
        if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip)) {
            std::string newName = "Scene " + std::to_string(nextSceneIndex++);
            sceneTabs.push_back(newName);
        }

        ImGui::EndTabBar();
    }
    ImGui::End();
}

void EditorUI::renderInspectorTabs() {
    ImGui::Begin("Inspector Panel");

    if (ImGui::BeginTabBar("InspectorTabs")) {
        if (ImGui::BeginTabItem("Properties")) {
            if (!m_selectedAssetName.empty()) {
                ImGui::Text("Selected: %s", m_selectedAssetName.c_str());
                    if (m_selectedAssetType == "Text") {
                        renderTextInspector(m_selectedAssetName);
                    } else if (m_selectedAssetType == "Character") {
                        renderCharacterInspector(m_selectedAssetName);
                    } else if (m_selectedAssetType == "Audio") {
                        renderAudioInspector(m_selectedAssetName);
                    }
            } else {
                ImGui::Text("No asset selected.");
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void EditorUI::renderAssetBrowser() {
    ImGui::Begin("Assets Panel");

    ImVec2 panelSize = ImGui::GetContentRegionAvail();
    float leftPaneWidth = panelSize.x * 0.3f;

    // Left: Folder tree
    ImGui::BeginChild("AssetFolders", ImVec2(leftPaneWidth, 0), true);
    renderFolderTree(m_assetsRoot, m_assetsRoot);
    ImGui::EndChild();

    ImGui::SameLine();

    // Right: Folder content preview
    ImGui::BeginChild("AssetPreview", ImVec2(0, 0), true);
    renderFolderPreview(m_selectedFolder);
    ImGui::EndChild();

    ImGui::End();
}

void EditorUI::renderFolderTree(const std::filesystem::path& path, const std::filesystem::path& base) {
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (!entry.is_directory()) continue;

        std::string label = entry.path().filename().string();
        bool isSelected = (entry.path() == m_selectedFolder);

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
        if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

        bool open = ImGui::TreeNodeEx(label.c_str(), flags);
        if (ImGui::IsItemClicked()) {
            m_selectedFolder = entry.path();
        }

        if (open) {
            renderFolderTree(entry.path(), base);
            ImGui::TreePop();
        }
    }
}


void EditorUI::renderFolderPreview(const std::filesystem::path& folder) {
    if (!std::filesystem::exists(folder)) {
        ImGui::Text("Folder not found.");
        return;
    }

    const float cellSize = 100.0f;
    const float padding = 10.0f;
    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = std::max(1, static_cast<int>(panelWidth / (cellSize + padding)));

    ImGui::Columns(columnCount, nullptr, false);

    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
        std::string name = entry.path().filename().string();

        if (entry.is_directory()) {
            ImGui::Button((std::string("📁 ") + name).c_str(), ImVec2(cellSize, cellSize * 0.7f));
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                m_selectedFolder = entry.path();  // Navigate into folder
            }
        } else {
            ImGui::Button((std::string("📄 ") + name).c_str(), ImVec2(cellSize, cellSize * 0.7f));
        }

        ImGui::NextColumn();
    }

    ImGui::Columns(1);

    // Drag-and-drop asset import support
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATHS")) {
            const char* path = static_cast<const char*>(payload->Data);
            if (path) {
                std::string filePath(path);
                if (endsWith(filePath, ".txt") || endsWith(filePath, ".json")) {
                    ImportManager::importAsset(filePath, AssetType::Text);
                } else if (endsWith(filePath, ".char")) {
                    ImportManager::importAsset(filePath, AssetType::Character);
                } else if (endsWith(filePath, ".mp3") || endsWith(filePath, ".wav")) {
                    ImportManager::importAsset(filePath, AssetType::Audio);
                } else {
                    std::cerr << "[DragDrop] Unsupported file type: " << filePath << "\n";
                }
            }
        }
        ImGui::EndDragDropTarget();
    }
}
