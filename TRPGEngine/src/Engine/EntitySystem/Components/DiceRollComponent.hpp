// DiceRollComponent.hpp
#pragma once
#include "ComponentBase.hpp"
#include "ComponentType.hpp"
#include <nlohmann/json.hpp>

class DiceRollComponent : public ComponentBase {
public:
  int sides = 6;
  int result = 0;

  ComponentType getType() const override { return ComponentType::DiceRoll; }
  std::string getID()  const override { return "dice"; }

  nlohmann::json toJson() const override {
    return {{"sides", sides}, {"result", result}};
  }
  static std::shared_ptr<DiceRollComponent> fromJson(const nlohmann::json& j) {
    auto c = std::make_shared<DiceRollComponent>();
    c->sides = j.value("sides",6);
    c->result = j.value("result",0);
    return c;
  }
};
