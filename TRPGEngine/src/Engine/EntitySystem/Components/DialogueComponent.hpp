#pragma once
#include "Engine/EntitySystem/ComponentBase.hpp"
#include "Engine/EntitySystem/ComponentType.hpp"
#include "Engine/EntitySystem/Entity.hpp"
#include <json.hpp>
#include <vector>
#include <string>

class DialogueComponent : public ComponentBase {
public:
    std::vector<std::string> lines;     // Dialogue lines (multiple)
    Entity speaker = INVALID_ENTITY;    // Character or narrator entity
    std::string targetFlowNode;         // Optional flow node transition if clicked
    bool advanceOnClick = true;         // Whether clicking continues the flow
    bool triggered = false;

    static ComponentType getStaticType() { return ComponentType::Dialogue; }
    ComponentType getType() const override { return getStaticType(); }
    std::string getID() const override { return "dialogue"; }

    nlohmann::json toJson() const override {
        return {
            { "lines", lines },
            { "speaker", int(speaker) },
            { "targetFlowNode", targetFlowNode },
            { "advanceOnClick", advanceOnClick },
            {"triggered", triggered}
        };
    }

    static std::shared_ptr<DialogueComponent> fromJson(const nlohmann::json& j) {
        auto c = std::make_shared<DialogueComponent>();
        c->lines = j.value("lines", std::vector<std::string>{});
        c->speaker = Entity(j.value("speaker", 0));
        c->targetFlowNode = j.value("targetFlowNode", "");
        c->advanceOnClick = j.value("advanceOnClick", true);
        c->triggered = j.value("triggered", false);
        return c;
    }
};