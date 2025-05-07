#include <iostream>

#include "Application.h"
#include "../UI/EditorUI.h"
#include "../Engine/EngineManager.h" 


Application::Application() {
    m_engineManager = new EngineManager();
    m_editorUI = new EditorUI();
    m_running = false;
}

Application::~Application() {
    delete m_editorUI;
    delete m_engineManager;
}

bool Application::initEngine() {
    return m_editorUI->init() && m_engineManager->initializeEngine();
}

void Application::run() {
    if (!initEngine()) {
        std::cerr << "Failed to initialize engine\n";
        return;
    }

    m_running = true;

    while (!m_editorUI->shouldClose()) {
        m_editorUI->beginFrame();

        m_editorUI->render();              // Renders the ImGui UI
        m_engineManager->tick();           // Updates engine (empty for now)

        m_editorUI->endFrame();
    }

    quit();
}

void Application::quit() {
    m_running = false;
}
