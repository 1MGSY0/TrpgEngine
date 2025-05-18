#pragma once

#include "Engine/Assets/AssetBase.h"
#include "Engine/Assets/AssetType.h"
#include <string>
#include <json.hpp>

class BackgroundComponent : public AssetBase {
public:
    std::string name;
    std::string texturePath;
    float scale = 1.0f;

    std::string getID() const override { return name; }

    AssetType getType() const override { return AssetType::Image; }

    nlohmann::json toJson() const override {
        return {
            {"name", name},
            {"texture", texturePath},
            {"scale", scale}
        };
    };

    static std::shared_ptr<BackgroundComponent> fromJson(const nlohmann::json& j) {
        auto comp = std::make_shared<BackgroundComponent>();
        comp->name = j.value("name", "");
        comp->texturePath = j.value("texture", "");
        comp->scale = j.value("scale", 1.0f);
        return comp;
    }
};