#include <imgui.h>
#include <fstream>
#include <memory>
#include <cstring>
#include <filesystem>
#include <misc/cpp/imgui_stdlib.h> 

#include "ScriptPanel.hpp"
#include "Engine/EntitySystem/Components/ScriptComponent.hpp"

#include "UI/ImGuiUtils/ImGuiUtils.hpp"
#include "UI/EditorUI.hpp"

void renderScriptInspector(std::shared_ptr<ScriptComponent> script) {
    if (!script) {
        ImGui::Text("No script selected.");
        return;
    }

    ImGui::InputText("Name", &script->name);
    ImGui::InputText("Script File", &script->scriptPath);
}