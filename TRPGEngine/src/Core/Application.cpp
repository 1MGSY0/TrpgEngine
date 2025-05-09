#include "Application.h"
#include <GLFW/glfw3.h>
#include <iostream>

// ImGui headers
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <Windows.h>

#include "UI/ImGUIUtils/ImGuiUtils.h"

Application::Application()
    : m_window(nullptr), m_editorUI(nullptr) {}

    Application::~Application() {
        if (m_window) {
            glfwDestroyWindow(m_window);  
            m_window = nullptr;
        }
        shutdown();
    }

void Application::shutdown() {
    glfwTerminate();
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
    glfwSwapInterval(1); // Enable VSync

    // Enable drag-and-drop on Windows
    HWND hwnd = glfwGetWin32Window(m_window);
    DragAcceptFiles(hwnd, TRUE);

    return true;
}

void Application::run() {
    if (!initWindow()) return;

    m_editorUI = new EditorUI(m_window);
    m_editorUI->init();

    initEngine();
    mainLoop();

    if (m_editorUI) {
        delete m_editorUI;
        m_editorUI = nullptr;
    }
}

void Application::mainLoop() {
    HWND hwnd = glfwGetWin32Window(m_window);

    while (!glfwWindowShouldClose(m_window)) {
        // Poll Windows messages
        MSG msg;
        while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_DROPFILES) {
                handleOSFileDrop(msg.hwnd);
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Normal ImGui frame logic
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
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}
