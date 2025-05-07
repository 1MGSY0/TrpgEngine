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
    void endFrame();
    void shutdown();
    bool shouldClose();
    void registerPanel(std::shared_ptr<IPanel> panel);

private:
    GLFWwindow* m_window;
    std::vector<std::shared_ptr<IPanel>> m_panels;
    
    void renderTabs(); 
    void renderMenuBar();  
    void showUnsavedChangesPopup();
    
    
};
