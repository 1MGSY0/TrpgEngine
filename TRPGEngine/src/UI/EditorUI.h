#pragma once
#include <vector>
#include <memory>

struct GLFWwindow;
class IPanel;

class EditorUI {
public:
    EditorUI(GLFWwindow* window);
    ~EditorUI();

    void init();
    void beginFrame();
    void render();
    void initDockLayout();

    void endFrame();
    void shutdown();
    bool shouldClose();
    void registerPanel(std::shared_ptr<IPanel> panel);

private:
    GLFWwindow* m_window;

    bool m_shouldBuildDockLayout = false;
    std::vector<std::shared_ptr<IPanel>> m_panels;

    void renderTabs(); 
    void renderFlowTabs();
    void renderSceneTabs();
    void renderInspectorTabs();
    void renderMenuBar();  
    void renderStatusBar();
    void showUnsavedChangesPopup();
    
    
};
