#pragma once
#include "Engine/EntitySystem/ComponentBase.hpp"
#include "Engine/EntitySystem/ComponentType.hpp"
#include <string>
#include <json.hpp>

class UIButtonComponent : public ComponentBase {
public:
    std::string text = "Button";
    std::string fontPath;
    std::string targetFlowNode;  // Optional: name or ID of next flow node
    std::string imagePath;       // Optional: button background image
    bool triggered = false;

    static ComponentType getStaticType() { return ComponentType::UIButton; }
    ComponentType getType() const override { return getStaticType(); }
    std::string getID() const override { return "ui_button"; }

    nlohmann::json toJson() const override {
        return {
            {"text", text},
            {"fontPath", fontPath},
            {"targetFlowNode", targetFlowNode},
            {"imagePath", imagePath},
            {"triggered", triggered}
        };
    }

    static std::shared_ptr<UIButtonComponent> fromJson(const nlohmann::json& j) {
        auto c = std::make_shared<UIButtonComponent>();
        c->text = j.value("text", "Button");
        c->fontPath = j.value("fontPath", "");
        c->targetFlowNode = j.value("targetFlowNode", "");
        c->imagePath = j.value("imagePath", "");
        c->triggered = j.value("triggered", false);
        return c;
    }
};