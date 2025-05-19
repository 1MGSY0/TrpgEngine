#pragma once

#include <memory>
struct GLFWwindow;
class EditorUI;

class Application {
public:
    Application();
    ~Application();

    void run();

private:
    bool initWindow();
    void shutdown();
    void mainLoop();
    void initEngine();
    void update();
    void render();

    GLFWwindow* m_window;
    std::unique_ptr<EditorUI> m_editorUI;
    bool m_isPlaying = false;
};
