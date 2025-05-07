#pragma once

struct GLFWwindow;

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

private:
    GLFWwindow* m_window;
    bool m_shouldBuildDockLayout = false;

    void renderTabs(); 
    void renderFlowTabs();
    void renderSceneTabs();
    void renderInspectorTabs();
    void renderMenuBar();  
};
