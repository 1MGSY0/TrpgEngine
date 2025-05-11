#pragma once
#include "Engine/Assets/AssetBase.h"

class ScriptComponent : public AssetBase {
public:
    std::string name;
    std::string scriptPath;

    std::string getID() const override { return name; }

    nlohmann::json toJson() const override {
        return {
            {"name", name},
            {"path", scriptPath}
        };
    }

    static std::shared_ptr<ScriptComponent> fromJson(const nlohmann::json& j) {
        auto comp = std::make_shared<ScriptComponent>();
        comp->name = j.value("name", "");
        comp->scriptPath = j.value("path", "");
        return comp;
    }

    AssetType getType() const override {
        return AssetType::Script;
    }
};
