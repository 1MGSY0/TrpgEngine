#pragma once

#include "Engine/Assets/AssetBase.h"
#include "Engine/Assets/AssetType.h"
#include <json.hpp>
#include <string>
#include <unordered_map>

class ObjectComponent : public AssetBase {
public:
    std::string name;
    std::unordered_map<std::string, std::string> stateImages;
    bool isHidden = false;

    std::string getID() const override { return name; }

    AssetType getType() const override { return AssetType::Image; }

    nlohmann::json toJson() const override {
        return {
            {"name", name},
            {"states", stateImages},
            {"isHidden", isHidden}
        };
    }

    static std::shared_ptr<ObjectComponent> fromJson(const nlohmann::json& j) {
        auto comp = std::make_shared<ObjectComponent>();
        comp->name = j.value("name", "");
        comp->stateImages = j.value("states", std::unordered_map<std::string, std::string>{});
        comp->isHidden = j.value("isHidden", false);
        return comp;
    }
};