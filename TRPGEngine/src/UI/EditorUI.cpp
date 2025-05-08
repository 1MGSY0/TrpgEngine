#include "EditorUI.h"

#include "AssetPanels/TextPanel.h"
#include "AssetPanels/CharactersPanel.h"
#include "AssetPanels/AudioPanel.h"
#include "ScenePanel/ScenePanel.h"

#include "UI/AssetPanels/TextPanel.h"
#include "UI/AssetPanels/CharactersPanel.h"
#include "UI/AssetPanels/AudioPanel.h"
#include "UI/Panels/ProjectPanel.h"
#include "UI/IPanel.h"

#include "Project/ProjectManager.h"
#include "Project/ResourceManager.h"
#include "Project/BuildSystem.h"
#include "Project/RuntimeLauncher.h"
#include "ImGUIUtils/ImGuiUtils.h"

#include <Windows.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <iostream>

#include <string>
#include <vector>
#include <imgui_internal.h>
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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable docking

    // Delay layout setup to next frame
    ImGui::GetIO().IniFilename = NULL; // disables .ini persistence
    m_shouldBuildDockLayout = true; // flag for one-time layout build
    
    io.Fonts->Clear();
    if (!io.Fonts->AddFontFromFileTTF("assets/fonts/InterVariable.ttf", 16.0f)) {
        std::cerr << "Failed to load font: assets/fonts/InterVariable.ttf" << std::endl;
    }
    io.FontGlobalScale = 1.5f; 

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
    style.ScaleAllSizes(2.0f);

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

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
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_MenuBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("MainDockSpace", nullptr, windowFlags);
    ImGui::PopStyleVar(2);

    // Docking area
    ImGuiID dockspace_id = ImGui::GetID("EditorDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_NoTabBar);

    if (m_shouldBuildDockLayout) {
        initDockLayout();
        m_shouldBuildDockLayout = false;
    }

    // Panels
    renderMenuBar();
    showUnsavedChangesPopup();
    renderFlowTabs();
    renderSceneTabs();
    renderInspectorTabs();
    renderTabs();

    ImGui::End();
}

void EditorUI::initDockLayout() {
    ImGuiID dockspace_id = ImGui::GetID("EditorDockspace");

    ImGui::DockBuilderRemoveNode(dockspace_id);         // clear any previous layout
    ImGui::DockBuilderAddNode(dockspace_id);            // create new dockspace
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetIO().DisplaySize);

    ImGuiID main_dock_id = dockspace_id;

    // Split right (Inspector)
    ImGuiID dock_right = ImGui::DockBuilderSplitNode(main_dock_id, ImGuiDir_Right, 0.20f, nullptr, &main_dock_id);

    // Split remaining horizontally top (Scene+Flow), bottom (Assets)
    ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(main_dock_id, ImGuiDir_Down, 0.30f, nullptr, &main_dock_id);
    ImGuiID dock_top = main_dock_id;

    // Split top horizontally left (Flow), center (Scene)
    ImGuiID dock_left = ImGui::DockBuilderSplitNode(dock_top, ImGuiDir_Left, 0.25f, nullptr, &dock_top);
    ImGuiID dock_center = dock_top;

    // Assign windows to regions
    ImGui::DockBuilderDockWindow("Flow Panel", dock_left);
    ImGui::DockBuilderDockWindow("Scene Panel", dock_center);
    ImGui::DockBuilderDockWindow("Inspector Panel", dock_right);
    ImGui::DockBuilderDockWindow("Assets Panel", dock_bottom);      
 
     ImGui::DockBuilderFinish(dockspace_id);
}

void EditorUI::endFrame() {
    if (!ImGui::GetCurrentContext())
        return;  // avoid draw calls if ImGui wasn't properly initialized
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
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New")) {
                if (ResourceManager::get().hasUnsavedChanges()) {
                    ImGui::OpenPopup("Unsaved Changes");
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
                    if (path.empty()) {
                        // just skip save, do nothing
                    } else {
                        ProjectManager::setCurrentProjectPath(path);
                        ProjectManager::save();
                        ResourceManager::get().setUnsavedChanges(false);
                    }
                }
            }

            if (ImGui::MenuItem("Save As...")) {
                std::string file = saveFileDialog();
                if (!file.empty()) {
                    ProjectManager::saveProjectToFile(file);  // use the full path
                    ProjectManager::setCurrentProjectPath(file);
                    ResourceManager::get().setUnsavedChanges(false);
                }
            }
            
            if (ImGui::BeginMenu("Build")) {
                if (ImGui::MenuItem("Build Project")) {
                    const std::string& projectPath = ProjectManager::getCurrentProjectPath();
            
                    if (projectPath.empty()) {
                        ImGui::OpenPopup("Missing Project Path");
                    } else {
                        std::string exportPath = openFileDialog();
                        if (!exportPath.empty()) {
                            bool success = BuildSystem::buildProject(projectPath, exportPath);
                            saveStatus = success ? "Build successful." : "Build failed.";
                        }
                    }
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginPopupModal("Missing Project Path", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("You must save the project before building.\n\n");
                ImGui::Separator();
                if (ImGui::Button("OK")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            if (ImGui::BeginMenu("Import")) {
                if (ImGui::BeginMenu("Assets")) {
                    if (ImGui::MenuItem("Text")) {
                        // Import Text logic here
                    }
                    if (ImGui::MenuItem("Character")) {
                        // Import Character logic here
                    }
                    if (ImGui::MenuItem("Audio")) {
                        // Import audio logic here
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Export")) {
                if (ImGui::MenuItem("Export Project")) {
                    // Export project logic here
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenu(); 
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo")) {
                // Undo logic here
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Select")) {
            if (ImGui::MenuItem("Select All")) {
                // Undo logic here
            }
            ImGui::EndMenu();
        }


        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Appearance")) {
                // Theme toggle or scaling coming soon
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Run")) {
            if (ImGui::MenuItem("Run Project")) {
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}


void EditorUI::showUnsavedChangesPopup() {
    if (ImGui::BeginPopupModal("Unsaved Changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("You have unsaved changes.\nSave before continuing?\n\n");
        ImGui::Separator();

        if (ImGui::Button("Save and Continue")) {
            std::string dir = ProjectManager::getCurrentProjectPath();
            if (dir.empty()) dir = "Projects/NewProject";
            ProjectManager::saveProjectToFile(dir);
            ResourceManager::get().setUnsavedChanges(false);

            if (actionAfterPrompt)
                ResourceManager::get().clear();
            else
                ProjectManager::loadProject(ProjectManager::getTempLoadPath());

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

void EditorUI::renderStatusBar() {
    static float timeSinceStatus = 0.0f;

    if (!saveStatus.empty()) {
        timeSinceStatus += ImGui::GetIO().DeltaTime;
        if (timeSinceStatus > 5.0f) {
            saveStatus.clear();
            timeSinceStatus = 0.0f;
            return;
        }
    } else {
        return;
    }

    ImVec2 windowSize = ImGui::GetWindowSize();
    float statusHeight = ImGui::GetTextLineHeightWithSpacing() + 10.0f;

    ImGui::SetCursorPosY(windowSize.y - statusHeight);
    ImGui::Separator();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);

    ImGui::BeginChild("StatusBar", ImVec2(0, statusHeight), false,
                      ImGuiWindowFlags_NoScrollbar |
                      ImGuiWindowFlags_NoScrollWithMouse |
                      ImGuiWindowFlags_NoDecoration);

    ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.4f, 1.0f), "%s", saveStatus.c_str());
    ImGui::EndChild();
}

void EditorUI::renderFlowTabs() {

    static std::string saveStatus;

    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    float height = displaySize.y * 0.65f;           // scale relative to screen height
    float width  = height * 0.415f;                 // maintain original ratio
    ImVec2 pos(0, displaySize.y * 0.10f);           // left side

    //ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    //ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Flow Panel", nullptr, flags);
    if (ImGui::BeginTabBar("FlowTabs")) {
        if (ImGui::BeginTabItem("Flow")) {
            ImGui::Text("Flow Graph content...");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Events")) {
            ImGui::Text("Event list...");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void EditorUI::renderSceneTabs() {

    static std::string saveStatus;
    static std::vector<std::string> sceneTabs = { "Scene 1" };  // initial scene
    static int nextSceneIndex = 2; // for naming new scenes

    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    float targetAspect = 867.0f / 628.0f;
    float height = displaySize.y * 0.6f;
    float width  = height * targetAspect;

    ImVec2 panelSize(width, height);
    ImVec2 panelPos(displaySize.x * 0.3f, displaySize.y * 0.15f); // center-ish

    //ImGui::SetNextWindowPos(panelPos, ImGuiCond_Always);
    //ImGui::SetNextWindowSize(panelSize, ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Scene Panel", nullptr, flags);
    if (ImGui::BeginTabBar("SceneTabs")) {
        // Render each tab
        for (int i = 0; i < sceneTabs.size(); ++i) {
            if (ImGui::BeginTabItem(sceneTabs[i].c_str())) {
                renderScenePanel();  // your function
                ImGui::EndTabItem();
            }
        }

        // Add "+" button styled as a tab
        if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip)) {
            std::string newName = "Scene " + std::to_string(nextSceneIndex++);
            sceneTabs.push_back(newName);
        }

        ImGui::EndTabBar();
    }
    ImGui::End();
}

void EditorUI::renderInspectorTabs() {

    static std::string saveStatus;

    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    float height = displaySize.y * 0.90f;
    float width  = height * 0.337f;
    ImVec2 pos(displaySize.x - width, displaySize.y * 0.10f);  // stick to right side

    //ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    //ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);


    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Inspector Panel", nullptr, flags);
    if (ImGui::BeginTabBar("Inspector")) {
        if (ImGui::BeginTabItem("Properties")) {
            ImGui::Text("Object properties...");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void EditorUI::renderTabs() {

    static std::string saveStatus;

    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    float width  = displaySize.x * 0.90f;
    float height = width / 3.81f;
    ImVec2 pos(displaySize.x * 0.05f, displaySize.y - height);  // bottom center

    //ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    //ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Assets Panel", nullptr, flags);
    if (ImGui::BeginTabBar("MainTabs")) {
        if (ImGui::BeginTabItem("Text")) {
            renderTextPanel();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Characters")) {
            renderCharactersPanel();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Items")) {
            
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Objects")) {
            
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Backgrounds")) {
            
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Audio")) {
            renderAudioPanel();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

