#pragma once
#include "Engine/EntitySystem/ComponentBase.hpp"
#include "Engine/EntitySystem/ComponentType.hpp"
#include <imgui.h>  
#include <glm/vec3.hpp>

class TransformComponent : public ComponentBase {
public:
    glm::vec3 position   = {0,0,0};
    glm::vec3 rotation   = {0,0,0};
    glm::vec3 scale      = {1,1,1};

    ComponentType getType() const override { return ComponentType::Transform; }
    std::string getID()  const override { return "transform"; }

    nlohmann::json toJson() const override {
      return {
        {"pos",{position.x,position.y,position.z}},
        {"rot",{rotation.x,rotation.y,rotation.z}},
        {"scale",{scale.x,scale.y,scale.z}}
      };
    }
    static std::shared_ptr<TransformComponent> fromJson(const nlohmann::json& j) {
      auto c = std::make_shared<TransformComponent>();
      auto p = j["pos"];
      c->position = {p[0],p[1],p[2]};
      /* ... similarly for rot/scale ... */
      return c;
    }
};