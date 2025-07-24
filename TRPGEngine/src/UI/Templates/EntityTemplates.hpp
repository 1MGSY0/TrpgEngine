#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include "Engine/EntitySystem/Components/Transform2DComponent.hpp"
#include "Engine/EntitySystem/Components/CharacterComponent.hpp"
#include "Engine/EntitySystem/Components/DialogueComponent.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/EntitySystem/Components/ChoiceComponent.hpp"
#include "Engine/EntitySystem/Components/DiceRollComponent.hpp"
#include "Engine/EntitySystem/Components/UIButtonComponent.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Entity.hpp"

struct EntityTemplate {
    std::string name;
    EntityType type;
    std::vector<std::shared_ptr<ComponentBase>> components;
};

inline const std::unordered_map<EntityType, EntityTemplate>& getEntityTemplateMap() {
    static std::unordered_map<EntityType, EntityTemplate> templates = {
        {
            EntityType::FlowNode,
            { "Flow Node", EntityType::FlowNode,{
                std::make_shared<Transform2DComponent>(),
                std::make_shared<FlowNodeComponent>()
            } }
        },
        {
            EntityType::Dialogue,
            { "Dialogue", EntityType::Dialogue,{
                std::make_shared<Transform2DComponent>(),
                std::make_shared<DialogueComponent>()
            } }
        },
        {
            EntityType::Character,
            { "Character", EntityType::Character,{
                std::make_shared<Transform2DComponent>(),
                std::make_shared<CharacterComponent>()
            } }
        },
        {
            EntityType::UIButton,
            { "UI Button", EntityType::UIButton,{
                std::make_shared<Transform2DComponent>(),
                std::make_shared<UIButtonComponent>()
            } }
        },
        {
            EntityType::Choice,
            { "Choice", EntityType::Choice,{
                std::make_shared<Transform2DComponent>(),
                std::make_shared<ChoiceComponent>()
            } }
        },
        {
            EntityType::DiceRoll,
            { "Dice Roll", EntityType::DiceRoll,{
                std::make_shared<Transform2DComponent>(),
                std::make_shared<DiceRollComponent>()
            } }
        }
    };
    return templates;
}

inline std::vector<std::pair<EntityType, std::string>> getEntityTypeNames() {
    return {
        { EntityType::FlowNode,  "Flow Node" },
        { EntityType::Dialogue,  "Dialogue" },
        { EntityType::Character, "Character" },
        { EntityType::UIButton,  "UI Button" },
        { EntityType::Choice,    "Choice" },
        { EntityType::DiceRoll,  "Dice Roll" }
    };
}
