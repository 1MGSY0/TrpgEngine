// FlowNodeComponent.hpp
#pragma once
#include "Engine/EntitySystem/ComponentBase.hpp"
#include "Engine/EntitySystem/ComponentType.hpp"
#include <string>
#include <memory>
#include <json.hpp>
#include <vector>

struct FlowOption {
  std::string label;
  int difficulty;
  Entity nextOnSuccess, nextOnFailure;
};

class FlowNodeComponent : public ComponentBase {
public:
  std::string name;
  ComponentType nextOnSuccess = ComponentType::Unknown;
  ComponentType nextOnFail    = ComponentType::Unknown;
  bool isStart = false;
  std::vector<Entity> dialogueEntities;
  std::vector<FlowOption> options;
  Entity nextNode;

  ComponentType getType() const override { return ComponentType::FlowNode; }
  std::string getID()  const override { return name; }

  nlohmann::json toJson() const override {
    return {
      {"name",name},
      {"onSuccess",int(nextOnSuccess)},
      {"onFail",int(nextOnFail)}
    };
  }
  static std::shared_ptr<FlowNodeComponent> fromJson(const nlohmann::json& j) {
    auto c = std::make_shared<FlowNodeComponent>();
    c->name = j.value("name","");
    c->nextOnSuccess = ComponentType(j.value("onSuccess",0));
    c->nextOnFail    = ComponentType(j.value("onFail",0));
    return c;
  }
};
