#pragma once
#include "Engine/EntitySystem/ComponentBase.hpp"
#include "Engine/EntitySystem/ComponentType.hpp"
#include <string>
#include <vector>
#include <json.hpp>

class DialogueComponent : public ComponentBase {
public:
    std::string name;
    std::vector<std::string> lines;
    bool isNarration = false;

    std::string getID() const override { return name; }
    ComponentType getType() const override { return ComponentType::Dialogue; }

    nlohmann::json toJson() const override {
        return {
            {"name", name},
            {"lines", lines},
            {"isNarration", isNarration}
        };
    };

    static std::shared_ptr<DialogueComponent> fromJson(const nlohmann::json& j) {
        auto comp = std::make_shared<DialogueComponent>();
        comp->name = j.value("name", "");
        comp->lines = j.value("lines", std::vector<std::string>{});
        comp->isNarration = j.value("isNarration", false);
        return comp;
    }
};