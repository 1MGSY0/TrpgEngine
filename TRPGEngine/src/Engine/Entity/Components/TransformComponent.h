#pragma once

#include "Engine/Assets/AssetBase.h"
#include "Engine/Assets/AssetType.h"
#include <imgui.h>
#include <json.hpp>

class TransformComponent : public AssetBase {
public:
    ImVec2 position = { 0.0f, 0.0f };
    ImVec2 scale = { 1.0f, 1.0f };
    float rotation = 0.0f;

    std::string getID() const override { return ""; }   //edit this

    AssetType getType() const override { return AssetType::Unknown; } //edit

    nlohmann::json toJson() const override {
        return {
            {"position", { position.x, position.y }},
            {"scale", { scale.x, scale.y }},
            {"rotation", rotation}
        };
    };
    
    std::shared_ptr<TransformComponent> fromJson(const nlohmann::json& j) {
        auto comp = std::make_shared<TransformComponent>();
        if (j.contains("position")) {
            comp->position.x = j["position"][0];
            comp->position.y = j["position"][1];
        }
        if (j.contains("scale")) {
            comp->scale.x = j["scale"][0];
            comp->scale.y = j["scale"][1];
        }
        comp->rotation = j.value("rotation", 0.0f);
        return comp;
    }
};
