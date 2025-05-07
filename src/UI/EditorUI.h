#pragma once

struct GLFWwindow;

class EditorUI {
public:
    EditorUI(GLFWwindow* window);
    ~EditorUI();

    void init();
    void beginFrame();
    void render(); 
    void endFrame();
    void shutdown();
    bool shouldClose();

private:
    GLFWwindow* m_window;

    void renderTabs(); 
    void renderMenuBar();  
};
