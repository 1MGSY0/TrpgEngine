#define NOMINMAX
#include "EditorUI.hpp"

#include <imgui.h>
#include <vector>
#include <algorithm> 
#include <iostream>
#include <filesystem>

#include "UI/ImGuiUtils/ImGuiUtils.hpp"
#include "Resources/ResourceManager.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/CharacterComponent.hpp"
#include "Engine/EntitySystem/Components/ScriptComponent.hpp"
#include "Engine/EntitySystem/ComponentRegistry.hpp"

#include "UI/ScenePanel/ScenePanel.hpp"
#include "UI/EntityInspectorPanel.hpp"


void EditorUI::renderFlowTabs() {
    static std::string saveStatus;

    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    float height = displaySize.y * 0.65f;
    float width  = height * 0.415f;
    ImVec2 pos(0, displaySize.y * 0.10f);

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
    static std::vector<std::string> sceneTabs = { "Scene 1" };
    static int nextSceneIndex = 2;

    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    float targetAspect = 867.0f / 628.0f;
    float height = displaySize.y * 0.6f;
    float width  = height * targetAspect;

    ImVec2 panelSize(width, height);
    ImVec2 panelPos(displaySize.x * 0.3f, displaySize.y * 0.15f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Scene Panel", nullptr, flags);
    if (ImGui::BeginTabBar("SceneTabs")) {
        for (int i = 0; i < sceneTabs.size(); ++i) {
            if (ImGui::BeginTabItem(sceneTabs[i].c_str())) {
                renderScenePanel();
                ImGui::EndTabItem();
            }
        }

        if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip)) {
            std::string newName = "Scene " + std::to_string(nextSceneIndex++);
            sceneTabs.push_back(newName);
        }

        ImGui::EndTabBar();
    }
    ImGui::End();
}

void EditorUI::renderInspectorTabs() {
    if (ImGui::Begin("Inspector")) {
        Entity selected = getSelectedEntity();
        renderEntityInspector(selected, EntityManager::get());
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

void EditorUI::renderFolderTree(const std::filesystem::path& path, const std::filesystem::path& base) {
    static std::filesystem::path hoveredFolder = m_selectedFolder;

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        std::string name = entry.path().filename().string();
        bool isFolder = entry.is_directory();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;
        if (entry.path() == m_selectedFolder)
            flags |= ImGuiTreeNodeFlags_Selected;

        if (!isFolder)
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

        bool open = ImGui::TreeNodeEx(name.c_str(), flags);

        if (ImGui::IsItemClicked()) {
            if (isFolder)
                m_selectedFolder = entry.path();
            else
                m_selectedFileName = entry.path().filename().string();
        }

        if (ImGui::IsItemHovered())
            hoveredFolder = entry.path();

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATHS")) {
                const char* dropped = static_cast<const char*>(payload->Data);
                if (dropped) {
                    auto dest = entry.path() / std::filesystem::path(dropped).filename();
                    std::error_code ec;
                    std::filesystem::rename(dropped, dest, ec);
                    if (!ec) {
                        ResourceManager::get().setUnsavedChanges(true);
                        setStatusMessage("Moved to: " + dest.string());
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (open && isFolder) {
            renderFolderTree(entry.path(), base);
            ImGui::TreePop();
        }
    }

    if (!s_pendingDroppedPaths.empty() && ImGui::IsWindowHovered()) {
        auto target = hoveredFolder.empty() ? m_selectedFolder : hoveredFolder;
        for (const auto& file : s_pendingDroppedPaths) {
            auto dest = target / std::filesystem::path(file).filename();
            std::error_code ec;
            std::filesystem::copy_file(file, dest, std::filesystem::copy_options::overwrite_existing, ec);
            if (!ec) {
                ResourceManager::get().setUnsavedChanges(true);
                setStatusMessage("Imported: " + dest.string());
            }
        }
        s_pendingDroppedPaths.clear();
    }
}


void EditorUI::renderFolderPreview(const std::filesystem::path& folder) {
    if (!std::filesystem::exists(folder)) {
        ImGui::Text("Folder not found.");
        return;
    }

    const float cellSize = 100.0f;
    const float padding = 10.0f;
    int columnCount = std::max(1, int(ImGui::GetContentRegionAvail().x / (cellSize + padding)));
    ImGui::Columns(columnCount, nullptr, false);

    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
        std::string name = entry.path().filename().string();
        std::string fullPath = entry.path().string();

        if (ImGui::Button(name.c_str(), ImVec2(cellSize, cellSize * 0.7f))) {
            m_selectedAssetName = entry.path().filename().string();
        }

        if (entry.is_regular_file() && ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("FILE_PATHS", fullPath.c_str(), fullPath.size() + 1);
            ImGui::TextUnformatted(name.c_str());
            ImGui::EndDragDropSource();
        }

        ImGui::NextColumn();
    }

    ImGui::Columns(1);

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATHS")) {
            const char* dropped = static_cast<const char*>(payload->Data);
            if (dropped) {
                auto dest = folder / std::filesystem::path(dropped).filename();
                std::error_code ec;
                std::filesystem::rename(dropped, dest, ec);
                if (!ec) {
                    ResourceManager::get().setUnsavedChanges(true);
                    setStatusMessage("Moved into preview: " + dest.string());
                }
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (!s_pendingDroppedPaths.empty() && ImGui::IsWindowHovered()) {
        for (const auto& path : s_pendingDroppedPaths) {
            auto dest = folder / std::filesystem::path(path).filename();
            std::error_code ec;
            std::filesystem::copy_file(path, dest, std::filesystem::copy_options::overwrite_existing, ec);
            if (!ec) {
                ResourceManager::get().setUnsavedChanges(true);
                setStatusMessage("Imported into preview: " + dest.string());
            }
        }
        s_pendingDroppedPaths.clear();
    }
}

