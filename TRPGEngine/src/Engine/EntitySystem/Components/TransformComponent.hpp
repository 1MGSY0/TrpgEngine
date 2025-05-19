#pragma once
#include "Engine/EntitySystem/ComponentBase.hpp"
#include "Engine/EntitySystem/ComponentType.hpp"
#include <imgui.h>  // For ImVec2

class TransformComponent : public ComponentBase {
public:
    ImVec2 position = ImVec2(0, 0);
    ImVec2 size = ImVec2(100, 100);  // default size

    std::string getID() const override { return "Transform"; }
    ComponentType getType() const override { return ComponentType::Transform; }

    nlohmann::json toJson() const override {
        return {
            {"position", { position.x, position.y }},
            {"size",     { size.x, size.y }}
        };
    }

    static std::shared_ptr<TransformComponent> fromJson(const nlohmann::json& j) {
        auto comp = std::make_shared<TransformComponent>();
        auto pos = j.value("position", std::vector<float>{0, 0});
        auto sz  = j.value("size",     std::vector<float>{100, 100});
        comp->position = ImVec2(pos[0], pos[1]);
        comp->size     = ImVec2(sz[0],  sz[1]);
        return comp;
    }
};
