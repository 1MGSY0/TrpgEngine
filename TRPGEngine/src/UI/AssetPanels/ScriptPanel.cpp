#include <imgui.h>
#include <fstream>
#include <memory>
#include <cstring>
#include <filesystem>
#include <misc/cpp/imgui_stdlib.h> 

#include "ScriptPanel.h"
#include "Engine/Entity/Components/ScriptComponent.h"

#include "UI/ImGuiUtils/ImGuiUtils.h"
#include "UI/EditorUI.h"

void renderScriptInspector(std::shared_ptr<ScriptComponent> script) {
    if (!script) {
        ImGui::Text("No script selected.");
        return;
    }

    ImGui::InputText("Name", &script->name);
    ImGui::InputText("Script File", &script->scriptPath);
}