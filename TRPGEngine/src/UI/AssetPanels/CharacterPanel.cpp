#include "CharacterPanel.h"
#include "Engine/Entity/Components/CharacterComponent.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Assets/ImportManager.h"

#include <imgui.h>
#include <imgui_stdlib.h>

static std::string name, spritePath;
static int hp = 100, mp = 50, str = 10;

void renderCharactersPanel() {
    ImGui::Text("Create Character");
    ImGui::InputText("Name", &name);
    ImGui::InputText("Sprite Path", &spritePath);
    ImGui::InputInt("HP", &hp);
    ImGui::InputInt("MP", &mp);
    ImGui::InputInt("Strength", &str);

    if (ImGui::Button("Add Character")) {
        auto character = std::make_shared<CharacterComponent>(name, name);
        character->setSpritePath(spritePath);
        character->setStat("HP", hp);
        character->setStat("MP", mp);
        character->setStat("STR", str);

        ResourceManager::get().addCharacter(name, character);

        name.clear();
        spritePath.clear();
    }

    ImGui::Separator();
    ImGui::Text("Characters:");
    for (const auto& c : ResourceManager::get().getAllCharacters()) {
        ImGui::BulletText("%s", c->getName().c_str());
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATHS")) {
            const char* path = static_cast<const char*>(payload->Data);
            ImportManager::importAsset(path, AssetType::Character);
        }
        ImGui::EndDragDropTarget();
    }
}

void renderCharacterInspector(const std::string& name) {
    auto character = ResourceManager::get().getCharacter(name);
    if (!character) return;

    ImGui::Text("Editing Character: %s", name.c_str());

    ImGui::InputText("Sprite Path", &character->getSpritePathRef());
    for (auto& [stat, value] : character->getStatsRef()) {
        ImGui::SliderInt(stat.c_str(), &value, 0, 100);
    }
}
