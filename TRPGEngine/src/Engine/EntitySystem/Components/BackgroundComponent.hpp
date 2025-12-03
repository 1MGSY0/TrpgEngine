#pragma once
#include "Engine/EntitySystem/ComponentBase.hpp"
#include "Engine/EntitySystem/ComponentType.hpp"
#include <string>
#include <memory>
#include <json.hpp>

struct BackgroundComponent : public ComponentBase {
    std::string assetPath;
    std::string image; // project-relative asset path, e.g. "Runtime/Assets/bg.png"

    std::string getID() const override { return "background"; }
    ComponentType getType() const override { return ComponentType::Background; }
    static ComponentType getStaticType() { return ComponentType::Background; }

    nlohmann::json toJson() const override {
        return { { "assetPath", assetPath }, { "image", image } };
    }

    static std::shared_ptr<BackgroundComponent> fromJson(const nlohmann::json& j) {
        auto comp = std::make_shared<BackgroundComponent>();
        comp->assetPath = j.value("assetPath", "");
        comp->image = j.value("image", "");
        return comp;
    }
};
