#pragma once

#include "Engine/Assets/AssetBase.h"
#include "Engine/Assets/AssetType.h"
#include <string>
#include <json.hpp>

class AudioComponent : public AssetBase {
public:
    std::string name;
    std::string audioPath;
    bool loop = false;

    std::string getID() const override { return name; }

    AssetType getType() const override { return AssetType::Character; }

    nlohmann::json toJson() const override {
        return {
            {"name", name},
            {"audioPath", audioPath},
            {"loop", loop}
        };
    };

    static std::shared_ptr<AudioComponent> fromJson(const nlohmann::json& j) {
        auto comp = std::make_shared<AudioComponent>();
        comp->audioPath = j.value("audioPath", "");
        comp->loop = j.value("loop", false);
        return comp;
    }
};