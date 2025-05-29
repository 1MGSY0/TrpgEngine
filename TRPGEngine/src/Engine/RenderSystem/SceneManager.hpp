#pragma once

#include <unordered_set>
#include <vector>
#include "Engine/EntitySystem/Entity.hpp"

class SceneManager {
public:
    static SceneManager& get();

    void setCurrentFlowNode(Entity node);
    Entity getCurrentFlowNode() const;

    void updateVisibleEntities();

    const std::unordered_set<Entity>& getVisibleEntities() const;
    const std::vector<Entity>& getUILayer() const;
    const std::vector<Entity>& getObjectLayer() const;
    Entity getCurrentEventEntity() const;

    void renderEditorScene();
    void renderRuntimeScene();

private:
    Entity m_currentFlowNode = INVALID_ENTITY;
    std::unordered_set<Entity> m_visibleEntities;
    std::vector<Entity> m_uiLayer;
    std::vector<Entity> m_objectLayer;
    Entity m_eventEntity = INVALID_ENTITY;
};
