#pragma once

#include "Engine/EntitySystem/ComponentBase.hpp"
#include "Engine/EntitySystem/ComponentType.hpp"
#include <json.hpp>
#include <string>
#include <unordered_map>

class CharacterComponent : public ComponentBase {
public:
    std::string name;
    std::unordered_map<std::string, int> stats;
    std::string iconImage = "Assets/Icons/no_image.png";
    std::unordered_map<std::string, std::string> stateImages;

    std::string getID() const override { return name; }

    ComponentType getType() const override { return ComponentType::Character; }

    nlohmann::json toJson() const override {
        return {
            {"name", name},
            {"stats", stats},
            {"icon", iconImage},
            {"states", stateImages}
        };
    }

    static std::shared_ptr<CharacterComponent> fromJson(const nlohmann::json& j) {
        auto comp = std::make_shared<CharacterComponent>();
        comp->name = j.value("name", "");
        comp->stats = j.value("stats", std::unordered_map<std::string, int>{});
        comp->iconImage = j.value("icon", "Assets/Icons/no_image.png");
        comp->stateImages = j.value("states", std::unordered_map<std::string, std::string>{});
        return comp;
    }

    ComponentType getType() const override {
        return ComponentType::Character;
    }
};