#pragma once
#include <string>
#include <filesystem>
#include <GLFW/glfw3.h>

class EditorUI {
public:
    EditorUI(GLFWwindow* window);
    ~EditorUI();

    void init();
    void beginFrame();
    void render();
    void endFrame();
    void shutdown();

    // Panels
    void renderMenuBar();
    void renderFlowTabs();
    void renderSceneTabs();
    void renderInspectorTabs();
    void renderAssetBrowser();
    void renderStatusBar();
    void showUnsavedChangesPopup();

    void initDockLayout();

private:
    GLFWwindow* m_window;
    bool m_shouldBuildDockLayout = false;

    std::string m_saveStatus;
    std::string m_selectedAssetName;
    std::string m_selectedAssetType;
    std::filesystem::path m_assetsRoot = "assets";        // base folder
    std::filesystem::path m_selectedFolder = m_assetsRoot; // Currently open folder

    void renderFolderTree(const std::filesystem::path& path, const std::filesystem::path& base);
    void renderFolderPreview(const std::filesystem::path& folder);

    bool m_showUnsavedPrompt = false;
    bool m_actionAfterPrompt = false;
    
};