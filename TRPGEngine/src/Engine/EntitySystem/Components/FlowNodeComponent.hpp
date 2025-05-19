#pragma once
#include "Engine/EntitySystem/ComponentBase.hpp"
#include "Engine/EntitySystem/ComponentType.hpp"

class FlowNodeComponent : public ComponentBase {
public:
    std::string nodeText;
    std::vector<int> linkedNodeIds;

    std::string getID() const override { return nodeText; }
    ComponentType getType() const override { return ComponentType::FlowNode; }

    nlohmann::json toJson() const override {
        return {
            {"nodeText", nodeText},
            {"linkedNodeIds", linkedNodeIds}
        };
    }

    static std::shared_ptr<FlowNodeComponent> fromJson(const nlohmann::json& j) {
        auto comp = std::make_shared<FlowNodeComponent>();
        comp->nodeText = j.value("nodeText", "");
        comp->linkedNodeIds = j.value("linkedNodeIds", std::vector<int>{});
        return comp;
    }
};
