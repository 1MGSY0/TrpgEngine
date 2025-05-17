#include "Entity.h"
#include "Components/AudioComponent.h"
#include "Components/BackgroundComponent.h"
#include "Components/CharacterComponent.h"
#include "Components/CutsceneComponent.h"
#include "Components/DialogueComponent.h"
#include "Components/ScriptComponent.h"
#include "Components/TransformComponent.h"

void Entity::fromJson(const nlohmann::json& j) {
    m_id = j.value("id", "");
    m_name = j.value("name", "");
    position = ImVec2(j["position"][0], j["position"][1]);
    size = ImVec2(j["size"][0], j["size"][1]);

    if (j.contains("components")) {
        const auto& comps = j["components"];

        if (comps.contains("AudioComponent")) {
            auto comp = std::make_shared<AudioComponent>();
            comp->fromJson(comps["AudioComponent"]);
            addComponent(comp);
        }
        if (comps.contains("BackgroundComponent")) {
            auto comp = std::make_shared<BackgroundComponent>();
            comp->fromJson(comps["BackgroundComponent"]);
            addComponent(comp);
        }
        if (comps.contains("CharacterComponent")) {
            auto comp = std::make_shared<CharacterComponent>();
            comp->fromJson(comps["CharacterComponent"]);
            addComponent(comp);
        }
        if (comps.contains("CutsceneComponent")) {
            auto comp = std::make_shared<CutsceneComponent>();
            comp->fromJson(comps["CutsceneComponent"]);
            addComponent(comp);
        }
        if (comps.contains("DialogueComponent")) {
            auto comp = std::make_shared<DialogueComponent>();
            comp->fromJson(comps["DialogueComponent"]);
            addComponent(comp);
        }
        if (comps.contains("ScriptComponent")) {
            auto comp = std::make_shared<ScriptComponent>();
            comp->fromJson(comps["ScriptComponent"]);
            addComponent(comp);
        }
        if (comps.contains("TransformComponent")) {
            auto comp = std::make_shared<TransformComponent>();
            comp->fromJson(comps["TransformComponent"]);
            addComponent(comp);
        }
    }
}