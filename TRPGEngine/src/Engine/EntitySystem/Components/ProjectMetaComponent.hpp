#pragma once
#include "Engine/EntitySystem/ComponentBase.hpp"
#include "Engine/EntitySystem/ComponentType.hpp"
#include <string>
#include <vector>
#include <memory>
#include <json.hpp>
#include "Engine/EntitySystem/Entity.hpp"

class ProjectMetaComponent : public ComponentBase {
public:
    std::string projectName = "New TRPG Project";
    std::string version = "1.0.0";
    std::string author = "Unknown";
    bool isActive = true;
    Entity startNode = INVALID_ENTITY;
    // All scenes (FlowNodes) part of this project
    std::vector<Entity> sceneNodes;

    std::string getID() const override { return projectName; }
    static ComponentType getStaticType() { return ComponentType::ProjectMetadata; }
    ComponentType getType() const override { return getStaticType(); }

    nlohmann::json toJson() const override {
        nlohmann::json j;
        j["projectName"] = projectName;
        j["version"] = version;
        j["author"] = author;
        j["isActive"] = isActive;
        j["startNode"] = int(startNode);
        std::vector<uint32_t> serializedSceneNodes;
        for (Entity e : sceneNodes) {
            serializedSceneNodes.push_back(e);
        }
        j["sceneNodes"] = serializedSceneNodes;

        return j;
    }

    static std::shared_ptr<ProjectMetaComponent> fromJson(const nlohmann::json& j) {
        auto comp = std::make_shared<ProjectMetaComponent>();
        comp->projectName = j.value("projectName", "Untitled");
        comp->version = j.value("version", "1.0.0");
        comp->author = j.value("author", "Unknown");
        comp->isActive = j.value("isActive", true);
        comp->startNode = Entity(j.value("startNode", 0));
        for (auto& id : j.value("sceneNodes", std::vector<uint32_t>{})) {
            comp->sceneNodes.push_back(static_cast<Entity>(id));
        }

        return comp;
    }
};