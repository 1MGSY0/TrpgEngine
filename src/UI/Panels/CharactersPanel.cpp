#include "CharactersPanel.h"
#include "Project/ResourceManager.h"
#include "Assets/Character.h"

#include <imgui.h>
#include <memory>

static char nameBuffer[128] = "";

void renderCharactersPanel() {
    ImGui::Text("Character Manager");

    ImGui::InputText("New Character Name", nameBuffer, IM_ARRAYSIZE(nameBuffer));
    if (ImGui::Button("Add Character")) {
        if (strlen(nameBuffer) > 0) {
            auto character = std::make_shared<Character>(nameBuffer);
            ResourceManager::get().addCharacter(character);
            nameBuffer[0] = '\0';
        }
    }

    ImGui::Separator();
    ImGui::Text("Characters:");

    const auto& characters = ResourceManager::get().getCharacters();
    for (const auto& c : characters) {
        ImGui::BulletText("%s", c->getName().c_str());
    }

    ImGui::Separator();
    if (ImGui::Button("Import Character File")) {
        // Simulate importing
        auto imported = std::make_shared<Character>("Imported_NPC");
        ResourceManager::get().addCharacter(imported);
    }
}
