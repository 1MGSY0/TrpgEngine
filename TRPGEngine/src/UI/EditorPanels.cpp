#define NOMINMAX
#include <glad/glad.h>
#include "EditorUI.hpp"

#include <imgui.h>
#include <vector>
#include <algorithm> 
#include <iostream>
#include <filesystem>

#include "UI/ImGuiUtils/ImGuiUtils.hpp"
#include "Resources/ResourceManager.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/ComponentRegistry.hpp"

#include "Engine/RenderSystem/SceneManager.hpp"
#include "UI/FlowPanel/Flowchart.hpp"
#include "UI/EntityInspectorPanel.hpp"

GLuint loadTextureFromFile(const char* path);

void EditorUI::renderFlowTabs() {
    static std::string saveStatus;

    // ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    // float height = displaySize.y * 0.65f;           // scale relative to screen height
    // float width  = height * 0.415f;                 // maintain original ratio
    // ImVec2 pos(0, displaySize.y * 0.10f);           // left side

    //ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    //ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Flow Panel", nullptr, flags);
    if (ImGui::BeginTabBar("FlowTabs")) {
        if (ImGui::BeginTabItem("Flow")) {
            m_flowChart.render(); 
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
    //Call ScenePanel rendering
    scenePanel.renderScenePanel();
}

void EditorUI::renderInspectorTabs() {
    if (ImGui::Begin("Inspector")) {
        Entity selected = getSelectedEntity();
        renderEntityInspector(selected);  
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

// GLuint loadTextureFromFile(const char* path) {
//     int width, height, channels;
//     unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
//     if (!data) {
//         std::cerr << "Failed to load image: " << path << std::endl;
//         return 0;
//     }

//     GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

//     GLuint textureID;
//     glGenTextures(1, &textureID);
//     glBindTexture(GL_TEXTURE_2D, textureID);

//     glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//     glGenerateMipmap(GL_TEXTURE_2D);

//     // Texture parameters (adjust as needed)
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 

//     stbi_image_free(data);
//     return textureID;
// }