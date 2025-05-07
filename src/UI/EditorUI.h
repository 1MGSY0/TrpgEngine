#pragma once

struct GLFWwindow;

class EditorUI {
public:
    EditorUI();
    ~EditorUI();

    bool init();              // Init window + ImGui
    void beginFrame();        // Start ImGui frame
    void render();            // Render ImGui content
    void endFrame();          // Finish frame + swap buffers
    bool shouldClose();       // Window close check

private:
    GLFWwindow* m_window;
};
