#pragma once

#include "Engine/EntitySystem/ComponentBase.hpp"
#include "Engine/EntitySystem/ComponentType.hpp"
#include <json.hpp>
#include <string>
#include <unordered_map>

struct UITriggerComponent : public ComponentBase {
    std::string label;                // Button label: "Start", "Run", etc.
    std::string actionTarget;        // FlowNode ID or event string

    std::string getID() const override { return "ui_trigger"; }
    ComponentType getType() const override { return ComponentType::UITrigger; }

    nlohmann::json toJson() const override {
        return { { "label", label }, { "actionTarget", actionTarget } };
    }

    static std::shared_ptr<UITriggerComponent> fromJson(const nlohmann::json& j) {
        auto comp = std::make_shared<UITriggerComponent>();
        comp->label = j.value("label", "");
        comp->actionTarget = j.value("actionTarget", "");
        return comp;
    }
};
