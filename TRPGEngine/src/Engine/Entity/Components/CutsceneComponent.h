#pragma once

#include "Engine/Assets/AssetBase.h"
#include "Engine/Assets/AssetType.h"
#include <json.hpp>
#include <string>
#include <unordered_map>

class CutsceneComponent : public AssetBase {
public:
    std::string name;
    std::string videoPath;
    float volume = 1.0f;

    std::string getID() const override { return name; }

    AssetType getType() const override { return AssetType::Video; }

    nlohmann::json toJson() const override {
        return {
            {"name", name},
            {"audioPath", videoPath},
            {"volume", volume},
        };
    }

    static std::shared_ptr<CutsceneComponent> fromJson(const nlohmann::json& j) {
        auto comp = std::make_shared<CutsceneComponent>();
        comp->name = j.value("name", "");
        comp->videoPath = j.value("stats", "");
        comp->volume = j.value("volume", 1.0f);
        return comp;
    }
};