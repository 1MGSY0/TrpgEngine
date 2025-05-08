#pragma once
#include "UI/EditorUI.h"

struct GLFWwindow;

class Application {
public:
    Application();
    ~Application();

    void run();

private:
    bool initWindow();
    void mainLoop();
    void shutdown();

    GLFWwindow* m_window;
    EditorUI* m_editorUI;

    // Dummy future stubs
    void initEngine();         
    void update();             
    void render();             
};
