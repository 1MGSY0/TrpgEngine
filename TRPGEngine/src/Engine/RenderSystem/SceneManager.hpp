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

    // Scene Panel render region in screen coordinates (set by ScenePanel)
    void setRenderRegion(float x, float y, float w, float h);
    float getRenderRegionX() const;
    float getRenderRegionY() const;
    float getRenderRegionW() const;
    float getRenderRegionH() const;

private:
    Entity m_currentFlowNode = INVALID_ENTITY;
    std::unordered_set<Entity> m_visibleEntities;
    std::vector<Entity> m_uiLayer;
    std::vector<Entity> m_objectLayer;
    Entity m_eventEntity = INVALID_ENTITY;

    // Last known Scene Panel render region reported by UI
    float m_renderRegionX = 0.0f;
    float m_renderRegionY = 0.0f;
    float m_renderRegionW = 0.0f;
    float m_renderRegionH = 0.0f;
};
