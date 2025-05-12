#pragma once
#include "Engine/EntitySystem/ComponentBase.h"
#include "Engine/EntitySystem/ComponentType.h"
#include <string>

class SceneMetadataComponent : public ComponentBase {
public:
    std::string sceneName;
    bool isActive = true;

    std::string getID() const override { return sceneName; }
    ComponentType getType() const override { return ComponentType::SceneMetadata; }

    nlohmann::json toJson() const override {
        return {
            {"sceneName", sceneName},
            {"isActive", isActive}
        };
    }

    static std::shared_ptr<SceneMetadataComponent> fromJson(const nlohmann::json& j) {
        auto comp = std::make_shared<SceneMetadataComponent>();
        comp->sceneName = j.value("sceneName", "");
        comp->isActive = j.value("isActive", true);
        return comp;
    }
};
