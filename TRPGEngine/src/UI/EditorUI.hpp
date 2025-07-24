#pragma once
#include <string>
#include <filesystem>
#include <imgui.h>
#include <imgui_internal.h>
#include <json.hpp>
#include <cstring>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h> 

#include "Core/Application.hpp"
#include "Core/EngineManager.hpp"

#include "UI/ScenePanel/ScenePanel.hpp"
#include "UI/FlowPanel/Flowchart.hpp"

#include "Engine/EntitySystem/Entity.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/ComponentRegistry.hpp"
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"

#include "Engine/RenderSystem/SceneManager.hpp"

#include "Resources/ResourceManager.hpp"

#include "Project/ProjectManager.hpp"
#include "Project/BuildSystem.hpp"

#include "ImGUIUtils/ImGuiUtils.hpp"
#include "Templates/EntityTemplates.hpp"

class Application;

class EditorUI {
public:
    EditorUI(GLFWwindow* window, Application* appInstance);
    ~EditorUI();

    static EditorUI* get();

    void init();
    void beginFrame();
    void render();
    void endFrame();
    void shutdown();
    void initDockLayout();

    // Panels
    void renderMenuBar();
    void renderFlowTabs();
    void renderSceneTabs();
    void renderInspectorTabs();
    void renderAssetBrowser();
    void renderStatusBar();
    void showUnsavedChangesPopup();

    void forceFolderRefresh();
    void handlePlatformEvents(); 
    void setStatusMessage(const std::string& message);
    void setSelectedEntity(Entity e);
    Entity getSelectedEntity() const { return m_selectedEntity; }

    static void glfwFileDropCallback(GLFWwindow* window, int count, const char** paths);
    
    const std::filesystem::path& getSelectedFolder() const { return m_selectedFolder; }
    const std::string& getSelectedFileName() const { return m_selectedFileName; }
    const std::filesystem::path& getAssetsRoot() const { return m_assetsRoot; }

private:
    GLFWwindow* m_window;
    Flowchart m_flowChart;
    ScenePanel scenePanel;

    Application* m_app = nullptr;
    Entity m_selectedEntity = INVALID_ENTITY;
    bool m_shouldBuildDockLayout = false;
    float m_statusTimer = 0.0f;


    std::string m_saveStatus;
    std::string m_selectedFileName;
    std::string m_selectedAssetName;
    std::filesystem::path m_assetsRoot = std::filesystem::current_path() / "Runtime";        // base folder
    std::filesystem::path m_selectedFolder = m_assetsRoot; // Currently open folder

    void renderFolderTree(const std::filesystem::path& path, const std::filesystem::path& base);
    void renderFolderPreview(const std::filesystem::path& folder);
    void renderRenamePopup();
    void renderNewEntityPopup();
    void renderProjectMetaPopup(Entity metaEntity);

    // UI state
    bool showRenamePopup = false;
    bool showNewEntityPopup = false;
    bool showProjectMetaPopup = false; 

    char newName[128] = "";
    char nameBuffer[128] = "";
    int selectedTemplate = 0;

    bool m_showUnsavedPrompt = false;
    bool m_actionAfterPrompt = false;
    bool m_forceRefresh = false;
    
};