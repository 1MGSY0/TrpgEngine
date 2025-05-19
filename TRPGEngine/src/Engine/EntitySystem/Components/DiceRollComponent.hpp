#pragma once
#include "Engine/EntitySystem/ComponentBase.hpp"
#include "Engine/EntitySystem/ComponentType.hpp"
#include <string>
#include <memory>
#include <json.hpp>

struct DiceRollComponent : public ComponentBase {
    int sides = 20;          // D20 roll
    int threshold = 10;      // Success if roll >= threshold
    std::string onSuccess;   // FlowNode to trigger
    std::string onFailure;

    std::string getID() const override { return "dice_roll"; }
    ComponentType getType() const override { return ComponentType::DiceRoll; }

    nlohmann::json toJson() const override {
        return {
            { "sides", sides },
            { "threshold", threshold },
            { "onSuccess", onSuccess },
            { "onFailure", onFailure }
        };
    }

    static std::shared_ptr<DiceRollComponent> fromJson(const nlohmann::json& j) {
        auto comp = std::make_shared<DiceRollComponent>();
        comp->sides = j.value("sides", 20);
        comp->threshold = j.value("threshold", 10);
        comp->onSuccess = j.value("onSuccess", "");
        comp->onFailure = j.value("onFailure", "");
        return comp;
    }
};
