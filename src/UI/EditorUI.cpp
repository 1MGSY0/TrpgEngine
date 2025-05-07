#include "EditorUI.h"
#include "Panels/TextPanel.h"
#include "Panels/CharactersPanel.h"
#include "Panels/AudioPanel.h"
#include "Project/ProjectManager.h"

#include <Windows.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <iostream>

EditorUI::EditorUI(GLFWwindow* window) : m_window(window) {}
static std::string saveStatus;

void EditorUI::init() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    ImGuiIO& io = ImGui::GetIO();
    
    io.Fonts->Clear();
    if (!io.Fonts->AddFontFromFileTTF("assets/fonts/InterVariable.ttf", 16.0f)) {
        std::cerr << "Failed to load font: assets/fonts/InterVariable.ttf" << std::endl;
    }
    io.FontGlobalScale = 1.0f; 
    
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_WindowBg]         = ImVec4(0.13f, 0.14f, 0.18f, 1.00f);
    colors[ImGuiCol_Header]           = ImVec4(0.20f, 0.22f, 0.28f, 1.00f);
    colors[ImGuiCol_HeaderHovered]    = ImVec4(0.27f, 0.29f, 0.35f, 1.00f);
    colors[ImGuiCol_HeaderActive]     = ImVec4(0.18f, 0.20f, 0.25f, 1.00f);
    colors[ImGuiCol_Button]           = ImVec4(0.20f, 0.22f, 0.28f, 1.00f);
    colors[ImGuiCol_ButtonHovered]    = ImVec4(0.35f, 0.37f, 0.42f, 1.00f);
    colors[ImGuiCol_ButtonActive]     = ImVec4(0.15f, 0.17f, 0.23f, 1.00f);
    colors[ImGuiCol_FrameBg]          = ImVec4(0.16f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_Tab]              = ImVec4(0.18f, 0.19f, 0.24f, 1.00f);
    colors[ImGuiCol_TabHovered]       = ImVec4(0.25f, 0.27f, 0.33f, 1.00f);
    colors[ImGuiCol_TabActive]        = ImVec4(0.20f, 0.22f, 0.30f, 1.00f);
    colors[ImGuiCol_TitleBg]          = ImVec4(0.10f, 0.11f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgActive]    = ImVec4(0.12f, 0.14f, 0.18f, 1.00f);

    style.FramePadding = ImVec2(10, 8);    
    style.ItemSpacing = ImVec2(12, 10);    
    style.WindowPadding = ImVec2(16, 12);  
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding      = 3.0f;
    style.TabRounding       = 3.0f;
    style.ScaleAllSizes(1.5f);

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

EditorUI::~EditorUI() {
    shutdown();  
}

void EditorUI::beginFrame() {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void EditorUI::render() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar |
                             ImGuiWindowFlags_NoBringToFrontOnFocus |
                             ImGuiWindowFlags_NoSavedSettings;
    
    ImGui::Begin("##MainWindow", nullptr, flags);  // No title, just content
    
    renderMenuBar();  // NEW
    renderTabs();     // Keep this
    ImGui::End();
}

void EditorUI::renderMenuBar() {
    static std::string saveStatus;

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save Project")) {
                if (ProjectManager::saveProject("MyProject.trpgproj")) {
                    saveStatus = "Project saved.";
                } else {
                    saveStatus = "Save failed.";
                }
            }
            if (ImGui::MenuItem("Load Project")) {
                if (ProjectManager::loadProject("MyProject.trpgproj")) {
                    saveStatus = "Project loaded.";
                } else {
                    saveStatus = "Load failed.";
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("System")) {
            if (ImGui::MenuItem("UI Settings")) {
                // TODO: Trigger UI settings window
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Run")) {
            if (ImGui::MenuItem("Preview Scene")) {
                // TODO: Call GameLogicSystem preview
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    if (!saveStatus.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.9f, 0.5f, 1.0f), "%s", saveStatus.c_str());
    }
}

void EditorUI::renderTabs() {

    static std::string saveStatus;

    if (ImGui::BeginTabBar("MainTabs")) {
        if (ImGui::BeginTabItem("Text")) {
            renderTextPanel();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Characters")) {
            renderCharactersPanel();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Audio")) {
            renderAudioPanel();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void EditorUI::endFrame() {
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(m_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    glClearColor(0.05f, 0.05f, 0.06f, 1.0f); // slightly darker, less eye strain
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(m_window);
}

void EditorUI::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
