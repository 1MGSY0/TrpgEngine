#pragma once

#include "Engine/Assets/AssetBase.h"
#include "Engine/Assets/AssetType.h"
#include <json.hpp>
#include <string>
#include <unordered_map>
#include "Engine/Entity/Components/ObjectComponent.h"

class CharacterComponent : public AssetBase {
public:
    std::string name;
    std::unordered_map<std::string, int> stats;
    std::string iconImage = "Assets/Icons/no_image.png";
    std::unordered_map<std::string, std::string> stateImages;
    std::unordered_map<std::string, std::shared_ptr<ObjectComponent>> inventoryItems;

    std::string getID() const override { return name; }

    AssetType getType() const override { return AssetType::Character; }

    nlohmann::json toJson() const override {
        nlohmann::json inventoryJson = nlohmann::json::object();
        for (const auto& item : inventoryItems) {
            if (item.second) {
                inventoryJson[item.first] = item.second->toJson();
            } else {
                inventoryJson[item.first] = nullptr;
            }
        }
        return {
            {"name", name},
            {"stats", stats},
            {"icon", iconImage},
            {"states", stateImages},
            {"inventory", inventoryJson}
        };
    }

    static std::shared_ptr<CharacterComponent> fromJson(const nlohmann::json& j) {
        auto comp = std::make_shared<CharacterComponent>();
        comp->name = j.value("name", "");
        comp->stats = j.value("stats", std::unordered_map<std::string, int>{});
        comp->iconImage = j.value("icon", "Assets/Icons/no_image.png");
        comp->stateImages = j.value("states", std::unordered_map<std::string, std::string>{});

        if (j.contains("inventory") && j["inventory"].is_object()) {
            for (auto& [key, value] : j["inventory"].items()) {
                if (!value.is_null()) {
                    auto obj = std::make_shared<ObjectComponent>();
                    obj->fromJson(value);  // your ObjectComponent::fromJson logic
                    comp->inventoryItems[key] = obj;
                } else {
                    comp->inventoryItems[key] = nullptr;
                }
            }
        }

        return comp;
    }
};