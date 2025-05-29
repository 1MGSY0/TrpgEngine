#include "EntityTemplates.hpp"
#include "Engine/EntitySystem/Components/TransformComponent.hpp"
#include "Engine/EntitySystem/Components/Transform2DComponent.hpp"
#include "Engine/EntitySystem/Components/CharacterComponent.hpp"
#include "Engine/EntitySystem/Components/DialogueComponent.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/EntitySystem/Components/ChoiceComponent.hpp"
#include "Engine/EntitySystem/Components/DiceRollComponent.hpp"

const std::vector<EntityTemplate>& getEntityTemplates() {
  static std::vector<EntityTemplate> templates = {
    {
      "Flow Node",
      {
        std::make_shared<Transform2DComponent>(),
        std::make_shared<FlowNodeComponent>()
      }
    },
    {
      "Dialogue",
      {
        std::make_shared<Transform2DComponent>(),
        std::make_shared<DialogueComponent>()
      }
    },
    {
      "Character",
      {
          std::make_shared<Transform2DComponent>(),
          std::make_shared<CharacterComponent>()
      }
    },
  };
  return templates;
}