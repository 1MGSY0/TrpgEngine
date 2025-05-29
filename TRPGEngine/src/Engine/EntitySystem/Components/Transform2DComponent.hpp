#pragma once
#include "Engine/EntitySystem/ComponentBase.hpp"
#include <include/glm.hpp>

class Transform2DComponent : public ComponentBase {
public:
    glm::vec2 position {0.0f, 0.0f};
    glm::vec2 size {100.0f, 100.0f}; 
    glm::vec2 scale {1.0f, 1.0f};
    float rotation = 0.0f; 

    static ComponentType getStaticType() { return ComponentType::Transform2D; }
    ComponentType getType() const override { return getStaticType(); }
    std::string getID() const override { return "transform2D";}

    nlohmann::json toJson() const override {
        return {
            { "position", { position.x, position.y } },
            { "size",     { size.x, size.y } },
            { "scale",    { scale.x, scale.y } },
            { "rotation", rotation }
        };
    }

    static std::shared_ptr<Transform2DComponent> fromJson(const nlohmann::json& j) {
        auto c = std::make_shared<Transform2DComponent>();
        if (j.contains("position") && j["position"].is_array() && j["position"].size() == 2)
            c->position = { j["position"][0], j["position"][1] };
        if (j.contains("size") && j["size"].is_array() && j["size"].size() == 2)
            c->size = { j["size"][0], j["size"][1] };
        if (j.contains("scale") && j["scale"].is_array() && j["scale"].size() == 2)
            c->scale = { j["scale"][0], j["scale"][1] };
        if (j.contains("rotation")) c->rotation = j["rotation"];
        return c;
    }
};