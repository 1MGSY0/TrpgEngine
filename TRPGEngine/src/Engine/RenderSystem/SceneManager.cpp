#include "SceneManager.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/GameplaySystem/GameInstance.hpp"
#include "Engine/GameplaySystem/FlowExecutor.hpp"
#include "Engine/RenderSystem/RenderSystem.hpp"

SceneManager& SceneManager::get() {
    static SceneManager instance;
    return instance;
}

void SceneManager::setCurrentFlowNode(Entity node) {
    m_currentFlowNode = node;
    updateVisibleEntities();
}

Entity SceneManager::getCurrentFlowNode() const {
    return m_currentFlowNode;
}

void SceneManager::updateVisibleEntities() {
    m_visibleEntities.clear();
    m_uiLayer.clear();
    m_objectLayer.clear();
    m_eventEntity = INVALID_ENTITY;

    auto& em = EntityManager::get();
    auto node = em.getComponent<FlowNodeComponent>(m_currentFlowNode);
    if (!node) return;

    for (auto e : node->characters)
        m_visibleEntities.insert(e);

    for (auto e : node->backgroundEntities)
        m_visibleEntities.insert(e);

    m_uiLayer = node->uiLayer;
    m_objectLayer = node->objectLayer;

    for (auto e : m_uiLayer)
        m_visibleEntities.insert(e);

    for (auto e : m_objectLayer)
        m_visibleEntities.insert(e);

    if (GameInstance::get().isRunning()) {
        int idx = FlowExecutor::get().currentEventIndex();
        if (idx < node->eventSequence.size()) {
            m_eventEntity = node->eventSequence[idx];
            m_visibleEntities.insert(m_eventEntity);
        }
    } else {
        for (auto e : node->eventSequence)
            m_visibleEntities.insert(e);
    }
}

const std::unordered_set<Entity>& SceneManager::getVisibleEntities() const {
    return m_visibleEntities;
}

const std::vector<Entity>& SceneManager::getUILayer() const {
    return m_uiLayer;
}

const std::vector<Entity>& SceneManager::getObjectLayer() const {
    return m_objectLayer;
}

Entity SceneManager::getCurrentEventEntity() const {
    return m_eventEntity;
}

void SceneManager::renderEditorScene() {
    for (Entity e : getVisibleEntities()) {
        RenderSystem::renderEntityEditor(e);
    }
}

void SceneManager::renderRuntimeScene() {
    for (Entity e : getObjectLayer()) {
        RenderSystem::renderEntityRuntime(e); // render 3D layer behind
    }

    Entity currentEvent = getCurrentEventEntity();
    if (currentEvent != INVALID_ENTITY)
        RenderSystem::renderEntityRuntime(currentEvent);

    for (Entity e : getUILayer()) {
        RenderSystem::renderEntityRuntime(e); // render UI on top
    }
}
