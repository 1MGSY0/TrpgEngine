#pragma once
#include "Engine/EntitySystem/ComponentBase.hpp"
#include "Engine/EntitySystem/ComponentType.hpp"
#include "Entity.hpp"
#include <vector>

class ChildrenComponent : public ComponentBase {
public:
    std::vector<Entity> children;

    std::string getID() const override { return "children"; }
    ComponentType getType() const override { return ComponentType::Children; }

    nlohmann::json toJson() const override {
        return { {"children", children} };
    }

    static std::shared_ptr<ChildrenComponent> fromJson(const nlohmann::json& j) {
        auto comp = std::make_shared<ChildrenComponent>();
        comp->children = j.value("children", std::vector<Entity>{});
        return comp;
    }
};
