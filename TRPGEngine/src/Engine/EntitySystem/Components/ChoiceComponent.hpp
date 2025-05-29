#pragma once
#include "Engine/EntitySystem/ComponentBase.hpp"
#include "Engine/EntitySystem/ComponentType.hpp"
#include <string>
#include <memory>
#include <json.hpp>

struct Choice {
  std::string text;
  ComponentType trigger; // FlowNode or some event type
};

class ChoiceComponent : public ComponentBase {
public:
  std::vector<Choice> options;

  ComponentType getType() const override { return ComponentType::Choice; }
  static ComponentType getStaticType() { return ComponentType::Choice; }
  std::string getID()  const override { return "choice"; }

  nlohmann::json toJson() const override {
    nlohmann::json arr = nlohmann::json::array();
    for (auto &o : options)
      arr.push_back({{"text",o.text},{"trigger",int(o.trigger)}});
    return {{"options",arr}};
  }
  static std::shared_ptr<ChoiceComponent> fromJson(const nlohmann::json& j) {
    auto c = std::make_shared<ChoiceComponent>();
    for (auto &o : j["options"]) {
      c->options.push_back({o["text"], ComponentType(o["trigger"].get<int>())});
    }
    return c;
  }
};
