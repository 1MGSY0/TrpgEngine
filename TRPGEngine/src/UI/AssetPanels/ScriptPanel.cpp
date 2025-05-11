#include "ScriptPanel.h"
#include "Engine/Entity/Components/ScriptComponent.h"
#include "Engine/Resources/ResourceManager.h"
#include "UI/ImGuiUtils/ImGuiUtils.h"
#include "UI/EditorUI.h"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h> 
#include <cstring>

void renderScriptInspector(const std::shared_ptr<ScriptComponent>& script) {
    if (!script) {
        ImGui::Text("No script selected.");
        return;
    }

    ImGui::InputText("Name", &script->name);
    ImGui::InputText("Script File", &script->scriptPath);
}

void renderAddScript() {
    auto script = std::make_shared<ScriptComponent>();
    script->name = "NewScript";
    script->scriptPath = "Assets/Scripts/new.lua";

    // Use selected folder from EditorUI
    auto folder = EditorUI::get()->getSelectedFolder();
    std::filesystem::create_directories(folder);
    std::string filePath = (folder / (script->name + ".lua")).string();

    // Add to resource manager
    ResourceManager::get().addAsset<ScriptComponent>(script);
    ResourceManager::get().setUnsavedChanges(true);
}