#pragma once

#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/Components/ScriptComponent.hpp"

inline void renderScriptInspector(const std::shared_ptr<ScriptComponent>& script) {
    if (!script) {
        ImGui::Text("No script selected.");
        return;
    }

    ImGui::InputText("Name", &script->name);
    ImGui::InputText("Script File", &script->scriptPath);
}