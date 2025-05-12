#include "EditorUI.hpp"
#include <imgui.h>
#include <imgui_internal.h>

void EditorUI::initDockLayout() {
    ImGuiID dockspace_id = ImGui::GetID("EditorDockspace");

    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetIO().DisplaySize);

    ImGuiID main_dock_id = dockspace_id;
    ImGuiID dock_right = ImGui::DockBuilderSplitNode(main_dock_id, ImGuiDir_Right, 0.20f, nullptr, &main_dock_id);
    ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(main_dock_id, ImGuiDir_Down, 0.30f, nullptr, &main_dock_id);
    ImGuiID dock_left = ImGui::DockBuilderSplitNode(main_dock_id, ImGuiDir_Left, 0.25f, nullptr, &main_dock_id);
    ImGuiID dock_center = main_dock_id;

    ImGui::DockBuilderDockWindow("Flow Panel", dock_left);
    ImGui::DockBuilderDockWindow("Scene Panel", dock_center);
    ImGui::DockBuilderDockWindow("Inspector Panel", dock_right);
    ImGui::DockBuilderDockWindow("Assets Panel", dock_bottom);

    ImGui::DockBuilderFinish(dockspace_id);
}
