#pragma once

#include <memory>
struct GLFWwindow;
class EditorUI;

class Application {
public:
    Application();
    ~Application();

    void run();
    void shutdown();

    bool isPlaying() const { return m_playing; }
    void togglePlayMode();

private:
    void initEngine();
    bool initWindow();
    void mainLoop();
    void update(float deltaTime);
    void render();

    bool m_playing = false;
    double m_lastFrameTime = 0.0;

    GLFWwindow* m_window = nullptr;
    std::unique_ptr<EditorUI> m_editorUI;
};
