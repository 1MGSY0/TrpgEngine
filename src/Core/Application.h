#include <string>
struct GLFWwindow;

class Application {
public:
    Application();
    ~Application();
    void run();

private:
    GLFWwindow* m_window;
    bool initWindow();
    void mainLoop();
    void shutdown();

    // UI functions
    void initImGui();
    void renderUI();
    void cleanupImGui();
};
