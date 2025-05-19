#pragma once
#include "Engine/EntitySystem/ComponentBase.hpp"
#include "Engine/EntitySystem/ComponentType.hpp"
#include <string>
#include <memory>
#include <json.hpp>
#include <unordered_map>

struct GameStateComponent : public ComponentBase {
    std::unordered_map<std::string, bool> flags;

    std::string getID() const override { return "game_state"; }
    ComponentType getType() const override { return ComponentType::GameState; }

    nlohmann::json toJson() const override {
        return flags;
    }

    static std::shared_ptr<GameStateComponent> fromJson(const nlohmann::json& j) {
        auto comp = std::make_shared<GameStateComponent>();
        for (auto& el : j.items()) {
            comp->flags[el.key()] = el.value().get<bool>();
        }
        return comp;
    }
};
