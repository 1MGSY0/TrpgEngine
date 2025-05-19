#pragma once
#include "Engine/EntitySystem/ComponentBase.hpp"
#include "Engine/EntitySystem/ComponentType.hpp"
#include <string>
#include <memory>
#include <json.hpp>

struct BackgroundComponent : public ComponentBase {
    std::string texturePath;

    std::string getID() const override { return "background"; }
    ComponentType getType() const override { return ComponentType::Background; }

    nlohmann::json toJson() const override {
        return { { "texturePath", texturePath } };
    }

    static std::shared_ptr<BackgroundComponent> fromJson(const nlohmann::json& j) {
        auto comp = std::make_shared<BackgroundComponent>();
        comp->texturePath = j.value("texturePath", "");
        return comp;
    }
};
