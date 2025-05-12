#pragma once
#include <string>
#include <filesystem>
#include <GLFW/glfw3.h>

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
    void renderEntityInspector(Entity entity, EntityManager& em);

    static void glfwFileDropCallback(GLFWwindow* window, int count, const char** paths);

    void setSelectedFolder(const std::filesystem::path& folder);
    
    const std::filesystem::path& getSelectedFolder() const { return m_selectedFolder; }
    const std::string& getSelectedFileName() const { return m_selectedFileName; }
    const std::filesystem::path& getAssetsRoot() const { return m_assetsRoot; }

private:
    GLFWwindow* m_window;
    Entity m_selectedEntity = INVALID_ENTITY;
    bool m_shouldBuildDockLayout = false;

    std::string m_saveStatus;
    std::string m_selectedFileName;
    std::filesystem::path m_assetsRoot = "Runtime";        // base folder
    std::filesystem::path m_selectedFolder = m_assetsRoot; // Currently open folder

    void renderFolderTree(const std::filesystem::path& path, const std::filesystem::path& base);
    void renderFolderPreview(const std::filesystem::path& folder);

    bool m_showUnsavedPrompt = false;
    bool m_actionAfterPrompt = false;
    bool m_forceRefresh = false;
    
public:
    void setSelectedEntity(Entity e) { m_selectedEntity = e; }
    Entity getSelectedEntity() const { return m_selectedEntity; }
};