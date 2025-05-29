#pragma once
#include "Engine/EntitySystem/ComponentBase.hpp"
#include "Engine/EntitySystem/ComponentType.hpp"
#include <string>
#include <memory>
#include <json.hpp>
#include <vector>

class FlowNodeComponent : public ComponentBase {
public:
    std::string name;             // Unique ID or label
    bool isStart = false;
    bool isEnd = false;

    Entity nextNode = INVALID_ENTITY; // For auto-advance

    std::vector<Entity> characters;   // Entities with Character + Transform
    std::vector<Entity> backgroundEntities; // Typically 1, with Transform
    std::vector<Entity> eventSequence; // Dialogue, Choice, DiceRoll, Trigger (in order)
    std::vector<Entity> uiLayer;       // Entities in UI Layer
    std::vector<Entity> objectLayer;   // Entities in 3D Layer

    ComponentType getType() const override { return ComponentType::FlowNode; }
    std::string getID() const override { return name; }
    static ComponentType getStaticType() { return ComponentType::FlowNode; }

    nlohmann::json toJson() const override {
        nlohmann::json j;
        j["name"] = name;
        j["isStart"] = isStart;
        j["isEnd"] = isEnd;
        j["nextNode"] = int(nextNode);
        j["characters"] = characters;
        j["backgrounds"] = backgroundEntities;
        j["eventSequence"] = eventSequence;
        j["uiLayer"] = uiLayer;
        j["objectLayer"] = objectLayer;
        return j;
    }

    static std::shared_ptr<FlowNodeComponent> fromJson(const nlohmann::json& j) {
        auto c = std::make_shared<FlowNodeComponent>();
        c->name = j.value("name", "");
        c->isStart = j.value("isStart", false);
        c->isEnd = j.value("isEnd", false);
        c->nextNode = Entity(j.value("nextNode", 0));

        for (auto& id : j.value("characters", std::vector<int>{}))
            c->characters.push_back(Entity(id));

        for (auto& id : j.value("backgrounds", std::vector<int>{}))
            c->backgroundEntities.push_back(Entity(id));

        for (auto& id : j.value("eventSequence", std::vector<int>{}))
            c->eventSequence.push_back(Entity(id));

        for (auto& id : j.value("uiLayer", std::vector<int>{}))
            c->uiLayer.push_back(Entity(id));

        for (auto& id : j.value("objectLayer", std::vector<int>{}))
            c->objectLayer.push_back(Entity(id));

        return c;
    }
};

