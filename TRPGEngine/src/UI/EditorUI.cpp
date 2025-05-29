#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL

#include <Windows.h>
#include <shellapi.h>

#include "EditorUI.hpp"

#include "UI/ScenePanel/ScenePanel.hpp"

#include "Resources/ResourceManager.hpp"
#include "Engine/EntitySystem/ComponentRegistry.hpp"
#include "Project/ProjectManager.hpp"
#include "Project/BuildSystem.hpp"
#include "Project/RuntimeLauncher.hpp"
#include "ImGUIUtils/ImGuiUtils.hpp"

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h> 
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>


#include <iostream>

std::vector<std::string> s_pendingDroppedPaths;

static std::string saveStatus;
static bool showUnsavedPrompt = false;
static bool actionAfterPrompt = false;

static EditorUI* s_instance = nullptr;

EditorUI* EditorUI::get() {
    return s_instance;
}

EditorUI::EditorUI(GLFWwindow* window, Application* appInstance)
    : m_window(window), m_app(appInstance) {}

EditorUI::~EditorUI() { shutdown(); }

void EditorUI::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void EditorUI::init() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::GetIO().IniFilename = NULL;
    m_shouldBuildDockLayout = true;

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    glfwSetWindowUserPointer(m_window, this);
    glfwSetDropCallback(m_window, EditorUI::glfwFileDropCallback);

    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF("assets/fonts/InterVariable.ttf", 16.0f);
    io.FontGlobalScale = 1.5f;

    applyCustomDarkTheme();

}

void EditorUI::beginFrame() {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void EditorUI::render() {
    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);

    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("MainDockSpace", nullptr, windowFlags);
    ImGui::PopStyleVar(2);

    // Ensure full screen background window has no padding
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGuiID dockspace_id = ImGui::GetID("EditorDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    ImGui::PopStyleVar();

    if (m_shouldBuildDockLayout) {
        initDockLayout();\
        m_shouldBuildDockLayout = false;
    }

    renderMenuBar();
    renderFlowTabs();        // "Flow"
    renderSceneTabs();       // "Scene"
    renderInspectorTabs();   // "Entity Inspector"
    renderAssetBrowser();    // "Assets"
    renderStatusBar();       // "Status Bar"

    showUnsavedChangesPopup();

    ImGui::End();
}


void EditorUI::endFrame() {
    if (!ImGui::GetCurrentContext())
        return;  // Prevent crash if ImGui wasn't initialized

    ImGui::Render();

    int display_w, display_h;
    glfwGetFramebufferSize(m_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.05f, 0.05f, 0.06f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(m_window);
}

void EditorUI::handlePlatformEvents() {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            glfwSetWindowShouldClose(m_window, true);
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void EditorUI::glfwFileDropCallback(GLFWwindow* window, int count, const char** paths) {
    if (count <= 0 || paths == nullptr) return;

    auto* editor = static_cast<EditorUI*>(glfwGetWindowUserPointer(window));
    if (!editor) return;

    for (int i = 0; i < count; ++i) {
        if (paths[i])
            s_pendingDroppedPaths.push_back(paths[i]);
    }
}



void EditorUI::setSelectedEntity(Entity e) {
    m_selectedEntity = e;

    if (EntityManager::get().hasComponent(e, ComponentType::FlowNode)) {
        SceneManager::get().setCurrentFlowNode(e);  // Only show current FlowNode
    }
}

void EditorUI::forceFolderRefresh() {
    m_forceRefresh = true; 
}