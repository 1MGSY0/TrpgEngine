#pragma once
#include <string>
#include <filesystem>
#include <GLFW/glfw3.h>

#include "UI/FlowPanel/Flowchart.hpp"

#include "Engine/EntitySystem/Entity.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"

class EditorUI {
public:
    EditorUI(GLFWwindow* window);
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

    static void glfwFileDropCallback(GLFWwindow* window, int count, const char** paths);

    void setSelectedFolder(const std::filesystem::path& folder);
    
    const std::filesystem::path& getSelectedFolder() const { return m_selectedFolder; }
    const std::string& getSelectedFileName() const { return m_selectedFileName; }
    const std::filesystem::path& getAssetsRoot() const { return m_assetsRoot; }

private:
    GLFWwindow* m_window;
    Flowchart m_flowChart;
    Entity m_selectedEntity = INVALID_ENTITY;
    bool m_shouldBuildDockLayout = false;
    float m_statusTimer = 0.0f;


    std::string m_saveStatus;
    std::string m_selectedFileName;
    std::string m_selectedAssetName;
    std::filesystem::path m_assetsRoot = "Runtime";        // base folder
    std::filesystem::path m_selectedFolder = m_assetsRoot; // Currently open folder

    void renderFolderTree(const std::filesystem::path& path, const std::filesystem::path& base);
    void renderFolderPreview(const std::filesystem::path& folder);
    void renderRenamePopup();
    void renderNewEntityPopup();

    // UI state
    bool showRenamePopup = false;
    bool showNewEntityPopup = false;

    char newName[128] = "";
    char nameBuffer[128] = "";
    int selectedTemplate = 0;

    bool m_showUnsavedPrompt = false;
    bool m_actionAfterPrompt = false;
    bool m_forceRefresh = false;
    
public:
    void setSelectedEntity(Entity e) { m_selectedEntity = e; }
    Entity getSelectedEntity() const { return m_selectedEntity; }
};