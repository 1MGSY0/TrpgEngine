#pragma once
#include "Engine/EntitySystem/ComponentBase.h"
#include "Engine/EntitySystem/ComponentType.h"
#include "Entity.hpp"

class ParentComponent : public ComponentBase {
public:
    Entity parentID = INVALID_ENTITY;

    std::string getID() const override { return std::to_string(parentID); }
    ComponentType getType() const override { return ComponentType::Parent; }

    nlohmann::json toJson() const override {
        return { { "parentID", parentID } };
    }

    static std::shared_ptr<ParentComponent> fromJson(const nlohmann::json& j) {
        auto comp = std::make_shared<ParentComponent>();
        comp->parentID = j.value("parentID", INVALID_ENTITY);
        return comp;
    }
};
