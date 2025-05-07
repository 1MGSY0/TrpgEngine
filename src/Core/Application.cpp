#include "Application.h"
#include <GLFW/glfw3.h>
#include <iostream>

// ImGui headers
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

Application::Application()
    : m_window(nullptr), m_editorUI(nullptr) {}

Application::~Application() {
    shutdown();
}

bool Application::initWindow() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    m_window = glfwCreateWindow(1280, 720, "TRPG Engine", nullptr, nullptr);
    
    if (!m_window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // V-Sync
    return true;
}

void Application::run() {
    if (!initWindow()) return;

    m_editorUI = new EditorUI(m_window);  
    m_editorUI->init();  

    initEngine();
    mainLoop();
}

void Application::mainLoop() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();

        m_editorUI->beginFrame();
        m_editorUI->render();
        m_editorUI->endFrame();

        glfwSwapBuffers(m_window);
    }
    delete m_editorUI;
}

void Application::shutdown() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

void Application::initEngine() {
    std::cout << "Engine Initialized (stub)\n";
}

void Application::update() {
    // Future: Tick subsystems, input, scripting, etc.
}

void Application::render() {
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}
