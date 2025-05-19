#include "EntityTemplates.hpp"
#include "Engine/EntitySystem/Components/TransformComponent.hpp"
#include "Engine/EntitySystem/Components/CharacterComponent.hpp"
#include "Engine/EntitySystem/Components/DialogueComponent.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/EntitySystem/Components/ChoiceComponent.hpp"
#include "Engine/EntitySystem/Components/DiceRollComponent.hpp"

const std::vector<EntityTemplate>& getEntityTemplates() {
  static std::vector<EntityTemplate> templates = {
    {
      "Start Node",
      {
        std::make_shared<TransformComponent>(),
        std::make_shared<FlowNodeComponent>()
      }
    },
    {
      "Dialogue Node",
      {
        std::make_shared<TransformComponent>(),
        std::make_shared<FlowNodeComponent>(),
        std::make_shared<DialogueComponent>()
      }
    },
    {
      "Choice Node",
      {
        std::make_shared<TransformComponent>(),
        std::make_shared<FlowNodeComponent>(),
        std::make_shared<ChoiceComponent>()
      }
    },
    {
      "Dice Roll Node",
      {
        std::make_shared<TransformComponent>(),
        std::make_shared<FlowNodeComponent>(),
        std::make_shared<DiceRollComponent>()
      }
    },
    {
      "End Node",
      {
        std::make_shared<TransformComponent>(),
        std::make_shared<FlowNodeComponent>()
      }
    }
  };
  return templates;
}