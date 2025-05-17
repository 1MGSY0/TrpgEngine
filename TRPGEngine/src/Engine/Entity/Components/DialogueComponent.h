#pragma once
#include "Engine/Assets/AssetBase.h"
#include "Engine/Assets/AssetType.h"
#include <string>
#include <vector>
#include <json.hpp>

class DialogueComponent : public AssetBase {
public:
    std::string name;
    std::vector<std::string> lines;
    bool isNarration = false;

    std::string getID() const override { return name; }

    AssetType getType() const override { return AssetType::Text; }

    nlohmann::json toJson() const override {
        return {
            {"name", name},
            {"lines", lines},
            {"isNarration", isNarration}
        };
    };

    std::shared_ptr<DialogueComponent> fromJson(const nlohmann::json& j) {
        auto comp = std::make_shared<DialogueComponent>();
        comp->name = j.value("name", "");
        comp->lines = j.value("lines", std::vector<std::string>{});
        comp->isNarration = j.value("isNarration", false);
        return comp;
    }
};