#pragma once
#include "Engine/EntitySystem/ComponentBase.hpp"
#include "Engine/EntitySystem/ComponentType.hpp"
#include <string>
#include <memory>
#include <json.hpp>
#include <imgui.h>  
#include <include/glm.hpp>

class TransformComponent : public ComponentBase {
public:
    glm::vec3 position {0.0f, 0.0f, 0.0f};
    glm::vec3 rotation {0.0f, 0.0f, 0.0f};
    glm::vec3 scale    {1.0f, 1.0f, 1.0f};

    ComponentType getType() const override { return ComponentType::Transform; }
    static ComponentType getStaticType()   { return ComponentType::Transform; }
    std::string getID() const override     { return "transform"; }

    nlohmann::json toJson() const override {
        return {
            { "pos",   { position.x, position.y, position.z } },
            { "rot",   { rotation.x, rotation.y, rotation.z } },
            { "scale", { scale.x,    scale.y,    scale.z } }
        };
    }

    static std::shared_ptr<TransformComponent> fromJson(const nlohmann::json& j) {
        auto c = std::make_shared<TransformComponent>();

        if (j.contains("pos") && j["pos"].is_array() && j["pos"].size() == 3)
            c->position = { j["pos"][0], j["pos"][1], j["pos"][2] };

        if (j.contains("rot") && j["rot"].is_array() && j["rot"].size() == 3)
            c->rotation = { j["rot"][0], j["rot"][1], j["rot"][2] };

        if (j.contains("scale") && j["scale"].is_array() && j["scale"].size() == 3)
            c->scale = { j["scale"][0], j["scale"][1], j["scale"][2] };

        return c;
    }
};