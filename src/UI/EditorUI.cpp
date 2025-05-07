#include "EditorUI.h"
#include "UI/Panels/TextPanel.h"
#include "UI/Panels/CharactersPanel.h"
#include "UI/Panels/AudioPanel.h"
#include "UI/Panels/ProjectPanel.h"
#include "UI/IPanel.h"

#include "Project/ProjectManager.h"
#include "Project/ResourceManager.h"
#include "ImGUIUtils/ImGuiUtils.h"

#include <Windows.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <iostream>
#include <memory>

static std::string saveStatus;
static bool showUnsavedPrompt = false;
static bool actionAfterPrompt = false;

EditorUI::EditorUI(GLFWwindow* window) : m_window(window) {}

EditorUI::~EditorUI() {
    shutdown();
}

void EditorUI::init() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    
    io.Fonts->Clear();
    if (!io.Fonts->AddFontFromFileTTF("assets/fonts/InterVariable.ttf", 16.0f)) {
        std::cerr << "Failed to load font: assets/fonts/InterVariable.ttf" << std::endl;
    }
    io.FontGlobalScale = 1.0f; 

    applyCustomDarkTheme(); // from ImGuiUtils
    
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
    style.ScaleAllSizes(3.0f);

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    //registerPanel(std::make_shared<TextPanel>());
    //registerPanel(std::make_shared<CharactersPanel>());
    //registerPanel(std::make_shared<AudioPanel>());
    registerPanel(std::make_shared<ProjectPanel>());
}

void EditorUI::registerPanel(std::shared_ptr<IPanel> panel) {
    m_panels.push_back(panel);
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
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoSavedSettings;
    
    ImGui::Begin("##MainWindow", nullptr, flags);
        renderMenuBar();
        showUnsavedChangesPopup();
        renderTabs();
    ImGui::End();
}

void EditorUI::endFrame() {
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(m_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    glClearColor(0.05f, 0.05f, 0.06f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(m_window);
}

void EditorUI::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void EditorUI::renderMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Project")) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New")) {
                    if (ResourceManager::get().hasUnsavedChanges()) {
                        showUnsavedPrompt = true;
                        actionAfterPrompt = true;
                    } else {
                        ResourceManager::get().clear();
                        ProjectManager::setCurrentProjectPath(""); 
                        ResourceManager::get().setUnsavedChanges(true);
                    }
                }

                if (ImGui::MenuItem("Open...")) {
                    std::string file = openFileDialog();
                    if (!file.empty()) {
                        if (ResourceManager::get().hasUnsavedChanges()) {
                            showUnsavedPrompt = true;
                            actionAfterPrompt = false;
                            ProjectManager::setTempLoadPath(file);  // <-- Store path to use after prompt
                        } else {
                            ProjectManager::loadProject(file);
                        }
                    }
                }

                if (ImGui::MenuItem("Save")) {
                    std::string path = ProjectManager::getCurrentProjectPath();
                    if (path.empty()) {
                        path = saveFileDialog();
                        if (path.empty()) return;  // Cancelled
                        ProjectManager::setCurrentProjectPath(path);
                    }
                
                    ProjectManager::save();
                    ResourceManager::get().setUnsavedChanges(false);
                }

                if (ImGui::MenuItem("Save As...")) {
                    std::string file = saveFileDialog();
                    if (!file.empty()) {
                        ProjectManager::saveProjectToFile(file);  // use the full path
                        ProjectManager::setCurrentProjectPath(file);
                        ResourceManager::get().setUnsavedChanges(false);
                    }
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("System")) {
                if (ImGui::MenuItem("UI Settings")) {
                    // Theme toggle or scaling coming soon
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenu();  // End "Project"
        }
        if (ImGui::BeginMenu("Run")) {
            if (ImGui::MenuItem("Preview Scene")) {
                // Start preview mode here
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}


void EditorUI::showUnsavedChangesPopup() {
    if (showUnsavedPrompt) {
        ImGui::OpenPopup("Unsaved Changes");
    }

    if (ImGui::BeginPopupModal("Unsaved Changes", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("You have unsaved changes.\nSave before continuing?\n\n");
        ImGui::Separator();

        if (ImGui::Button("Save and Continue")) {
            std::string dir = ProjectManager::getCurrentProjectPath();
            if (dir.empty()) dir = "Projects/NewProject";
            ProjectManager::saveProjectToFile(dir);
            ResourceManager::get().setUnsavedChanges(false);
        
            if (actionAfterPrompt) {
                ResourceManager::get().clear();
            } else {
                ProjectManager::loadProject(ProjectManager::getTempLoadPath());
            }
        
            showUnsavedPrompt = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Discard Changes")) {
            ResourceManager::get().setUnsavedChanges(false);

            if (actionAfterPrompt)
                ResourceManager::get().clear();
            else
                ProjectManager::loadProject(ProjectManager::getTempLoadPath());

            showUnsavedPrompt = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            showUnsavedPrompt = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
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

