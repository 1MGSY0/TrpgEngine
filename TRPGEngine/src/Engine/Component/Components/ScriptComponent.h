#pragma once
#include "Engine/Entity/ComponentBase.h"
#include "Engine/Entity/ComponentType.h"

class ScriptComponent : public ComponentBase {
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

    ComponentType getType() const override {
        return ComponentType::Script;
    }
};
