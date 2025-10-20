#define GLFW_INCLUDE_NONE 
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <filesystem> 
#include <Windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Application.hpp"
#include "EngineManager.hpp"
#include "Engine/GameplaySystem/GameInstance.hpp"
#include "UI/EditorUI.hpp"
#include "UI/ImGuiUtils/ImGuiUtils.hpp"

Application::Application()
    : m_window(nullptr) {
    std::cout << "[Application] Constructed\n";
}

Application::~Application() {
    std::cout << "[Application] Destructing...\n";
    shutdown();
}

void Application::shutdown() {
    std::cout << "[Application] Shutting down...\n";

    if (m_editorUI) {
        std::cout << "[Application] Shutting down Editor UI\n";
        m_editorUI->shutdown();
    }

    if (m_window) {
        std::cout << "[Application] Destroying window\n";
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }

    std::cout << "[Application] Terminating GLFW\n";
    glfwTerminate();
}

bool Application::initWindow() {
    std::cout << "[Application] Initializing window...\n";

    if (!glfwInit()) {
        std::cerr << "[Error] Failed to initialize GLFW\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    m_window = glfwCreateWindow(1280, 720, "TRPG Engine", nullptr, nullptr);
    if (!m_window) {
        std::cerr << "[Error] Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "[Error] Failed to initialize GLAD\n";
        return false;
    }

    // Setup OpenGL state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set initial viewport size
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    glViewport(0, 0, width, height);

    // Handle future resizing
    glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow*, int w, int h) {
        glViewport(0, 0, w, h);
    });

    const auto* version = glGetString(GL_VERSION);
    std::cout << "[OpenGL] Version: " << (version ? reinterpret_cast<const char*>(version) : "Unknown") << "\n";

    glfwSwapInterval(1);  // VSync
    glfwSetWindowAttrib(m_window, GLFW_RESIZABLE, GLFW_TRUE);

    std::cout << "[Application] Window initialized successfully.\n";
    return true;
}

void Application::run() {
    std::cout << "[Application] Starting run loop...\n";

    if (!initWindow()) {
        std::cerr << "[Error] Window initialization failed. Aborting.\n";
        return;
    }

    // Set working directory to the .exe directory
    {
        char exePath[MAX_PATH];
        GetModuleFileNameA(nullptr, exePath, MAX_PATH);
        std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
        std::filesystem::current_path(exeDir);
        std::cout << "[WorkingDir] Set to: " << exeDir << "\n";
    }

    std::cout << "[Application] Creating Editor UI\n";
    m_editorUI = std::make_unique<EditorUI>(m_window, this);
    m_editorUI->init();

    initEngine();
    mainLoop();

    std::cout << "[Application] Exiting main loop. Cleaning up.\n";
    m_editorUI.reset();
}

void Application::initEngine() {
    EngineManager::get().initialize();
}

void Application::mainLoop() {
    std::cout << "[Application] Entering main loop\n";

    while (!glfwWindowShouldClose(m_window)) {
        double start = glfwGetTime();
        float deltaTime = static_cast<float>(start - m_lastFrameTime);
        m_lastFrameTime = start;

        update(deltaTime);
        render();

        glfwPollEvents();  // Essential to prevent freezing

        double end = glfwGetTime();
        double frameDuration = end - start;

        if (frameDuration < targetFrameTime) {
            std::this_thread::sleep_for(std::chrono::duration<double>(targetFrameTime - frameDuration));
        }
    }

    std::cout << "[Application] Main loop exited\n";
}

void Application::update(float deltaTime) {
    // Game and editor logic update
    // std::cout << "[Application] Updating (dt=" << deltaTime << ")\n";

    if (GameInstance::get().isRunning()) {
        GameInstance::get().update(deltaTime);
    }
}

void Application::render() {
    // std::cout << "[Application] Rendering frame\n";

    // Clear the screen before rendering UI
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Begin UI frame
    m_editorUI->beginFrame();

    // Let EditorUI handle all panels (including SceneManager inside ScenePanel)
    m_editorUI->render();

    // Finalize and draw UI
    m_editorUI->endFrame();
}

void Application::togglePlayMode() {
    m_playing = !m_playing;
    std::cout << "[Application] Play mode: " << (m_playing ? "ON" : "OFF") << "\n";

    if (m_playing) {
        GameInstance::get().startGame();
    } else {
        GameInstance::get().reset();
    }
}
