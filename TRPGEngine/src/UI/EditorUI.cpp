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
    std::cout << "[EditorUI] Shutting down ImGui context...\n";
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    std::cout << "[EditorUI] Shutdown complete.\n";
}

void EditorUI::init() {
    std::cout << "[EditorUI] Initializing ImGui context...\n";
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
    io.Fonts->AddFontFromFileTTF("Assets/fonts/InterVariable.ttf", 16.0f);
    io.FontGlobalScale = 1.5f;

    std::cout << "[EditorUI] ImGui initialized. Custom dark theme applied.\n";
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

    bool began = ImGui::Begin("MainDockSpace", nullptr, windowFlags);
    ImGui::PopStyleVar(2);

    // If Begin() returns false you still must End() immediately.
    if (!began) { ImGui::End(); return; }

    // RAII guard: no matter what happens below, End() will be called.
    struct EndOnExit { ~EndOnExit(){ ImGui::End(); } } _ensure_end;

    // (keep the rest of your code exactly as-is)
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGuiID dockspace_id = ImGui::GetID("EditorDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    ImGui::PopStyleVar();

    if (m_shouldBuildDockLayout) {
        std::cout << "[EditorUI] Building initial dock layout...\n";
        initDockLayout();
        m_shouldBuildDockLayout = false;  // ← no stray backslash now
    }

    renderMenuBar();
    renderFlowTabs();
    renderSceneTabs();
    renderInspectorTabs();
    renderAssetBrowser();
    renderStatusBar();

    showUnsavedChangesPopup();
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
            std::cout << "[EditorUI] File dropped: " << paths[i] << "\n";
    }
}



void EditorUI::setSelectedEntity(Entity e) {
    m_selectedEntity = e;
    std::cout << "[EditorUI] Selected entity ID: " << e << "\n";
    if (EntityManager::get().hasComponent(e, ComponentType::FlowNode)) {
        SceneManager::get().setCurrentFlowNode(e);  // Only show current FlowNode
        std::cout << "[EditorUI] FlowNode component found. Set as current FlowNode.\n";
    }
}

void EditorUI::forceFolderRefresh() {
    std::cout << "[EditorUI] Force-refreshing folder view...\n";
    m_forceRefresh = true; 
}

void EditorUI::StartNewProjectFlow_() {
    std::cout << "[EditorUI] StartNewProjectFlow_: clearing session\n";
    ResourceManager::get().clear();
    EntityManager::get().clear();
    ProjectManager::setCurrentProjectPath("");

    const std::string defaultProjectName = "Untitled";
    const std::string defaultProjectPath = "Runtime/" + defaultProjectName + ".trpgproj";
    std::error_code ec;
    std::filesystem::create_directories("Runtime", ec);
    if (ec) std::cout << "[EditorUI] create_directories Runtime error: " << ec.message() << "\n";

    std::cout << "[EditorUI] CreateNewProject(name=" << defaultProjectName
              << ", path=" << defaultProjectPath << ")\n";
    if (!ProjectManager::CreateNewProject(defaultProjectName, defaultProjectPath)) {
        std::cout << "[EditorUI] CreateNewProject FAILED\n";
        setStatusMessage("Failed to create new project.");
        return;
    }

    ProjectManager::setCurrentProjectPath(defaultProjectPath);
    ResourceManager::get().setUnsavedChanges(true);
    std::cout << "[EditorUI] New project created -> requestProjectInfoPrompt()\n";
    ProjectManager::requestProjectInfoPrompt();   // deferred-safe
    setStatusMessage("New project created. Please fill in project info.");
}