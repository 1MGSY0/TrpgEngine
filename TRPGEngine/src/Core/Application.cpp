#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL

#include <iostream>
#include <memory>

#include <Windows.h>
#include "Application.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <shellapi.h>

#include "UI/EditorUI.hpp"
#include "UI/ImGUIUtils/ImGuiUtils.hpp"

// Use unique_ptr to manage EditorUI automatically
Application::Application()
    : m_window(nullptr) {}

Application::~Application() {
    shutdown();
}

void Application::shutdown() {
    if (m_editorUI)
        m_editorUI->shutdown();  // Shutdown ImGui and UI systems

    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }

    glfwTerminate();  // Final GLFW cleanup
}

bool Application::initWindow() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    m_window = glfwCreateWindow(1280, 720, "TRPG Engine", nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return false;
    }

    const auto* version = glGetString(GL_VERSION);
    std::cout << "OpenGL Version: " 
              << (version ? reinterpret_cast<const char*>(version) : "Unknown") 
              << std::endl;

    glfwSwapInterval(1);  // Enable VSync
    glfwSetWindowAttrib(m_window, GLFW_RESIZABLE, GLFW_TRUE);

    return true;
}

void Application::run() {
    if (!initWindow())
        return;

    m_editorUI = std::make_unique<EditorUI>(m_window);
    m_editorUI->init();

    initEngine();
    mainLoop();

    // UI cleanup via RAII
    m_editorUI.reset();
}

void Application::mainLoop() {
    while (!glfwWindowShouldClose(m_window)) {
        m_editorUI->handlePlatformEvents();
        m_editorUI->beginFrame();
        m_editorUI->render();
        m_editorUI->endFrame();
    }
}

void Application::initEngine() {
    std::cout << "Engine Initialized (stub)\n";
}

void Application::update() {
    // Future: Tick subsystems, input, scripting, etc.
}

void Application::render() {
    // No need to clear manually if ImGui handles it
    // If needed, move glClear inside endFrame
}

