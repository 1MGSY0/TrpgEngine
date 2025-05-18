#include "EditorUI.hpp"
#include <imgui.h>
#include <imgui_internal.h>

void EditorUI::initDockLayout() {
    ImGuiID dockspace_id = ImGui::GetID("EditorDockspace");

    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetIO().DisplaySize);

    ImGuiID main_dock_id = dockspace_id;

    ImGuiID dock_right   = ImGui::DockBuilderSplitNode(main_dock_id, ImGuiDir_Right,  0.20f, nullptr, &main_dock_id);
    ImGuiID dock_bottom  = ImGui::DockBuilderSplitNode(main_dock_id, ImGuiDir_Down,   0.25f, nullptr, &main_dock_id);
    ImGuiID dock_left    = ImGui::DockBuilderSplitNode(main_dock_id, ImGuiDir_Left,   0.25f, nullptr, &main_dock_id);
    ImGuiID dock_status  = ImGui::DockBuilderSplitNode(dock_bottom,  ImGuiDir_Down,   0.10f, nullptr, &dock_bottom);
    ImGuiID dock_center  = main_dock_id;

    ImGui::DockBuilderDockWindow("Flow Panel", dock_left);
    ImGui::DockBuilderDockWindow("Scene Panel", dock_center);
    ImGui::DockBuilderDockWindow("Inspector", dock_right);
    ImGui::DockBuilderDockWindow("Assets Panel", dock_bottom);
    ImGui::DockBuilderDockWindow("StatusBar", dock_status);

    ImGui::DockBuilderGetNode(dock_left)->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
    ImGui::DockBuilderGetNode(dock_right)->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
    ImGui::DockBuilderGetNode(dock_bottom)->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
    ImGui::DockBuilderGetNode(dock_status)->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
    ImGui::DockBuilderGetNode(dock_center)->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;


    ImGui::DockBuilderFinish(dockspace_id);
}
