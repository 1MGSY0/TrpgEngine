#pragma once
#include "Engine/EntitySystem/ComponentBase.hpp"
#include "Engine/EntitySystem/ComponentType.hpp"
#include <string>
#include <vector>
#include <json.hpp>

class DialogueComponent : public ComponentBase {
public:
    std::vector<std::string> lines;
    ComponentType getType() const override { return ComponentType::Dialogue; }
    std::string getID()  const override { return "dialogue"; }

    nlohmann::json toJson() const override {
      return { {"lines", lines} };
    }
    static std::shared_ptr<DialogueComponent> fromJson(const nlohmann::json& j) {
      auto c = std::make_shared<DialogueComponent>();
      c->lines = j.value("lines", std::vector<std::string>{});
      return c;
    }
};
