#include "SceneManager.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/GameplaySystem/GameInstance.hpp"
#include "Engine/GameplaySystem/FlowExecutor.hpp"
#include "Engine/RenderSystem/RenderSystem.hpp"
#include <iostream>
#include <unordered_set>
// Auto-select default scene for validation/preview
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"
#include "Project/ProjectManager.hpp"

SceneManager& SceneManager::get() {
    static SceneManager instance;
    return instance;
}

void SceneManager::setCurrentFlowNode(Entity node) {
    m_currentFlowNode = node;
    updateVisibleEntities();
    //std::cout << "[SceneManager] setCurrentFlowNode -> node: " << (unsigned)node << std::endl;
}

Entity SceneManager::getCurrentFlowNode() const {
    return m_currentFlowNode;
}

void SceneManager::updateVisibleEntities() {
    auto& em = EntityManager::get();
    m_visibleEntities.clear();
    m_uiLayer.clear();
    m_objectLayer.clear();

    Entity node = m_currentFlowNode;
    auto fn = em.getComponent<FlowNodeComponent>(node);
    if (!fn) {
        return;
    }

    for (auto e : fn->characters)
        m_visibleEntities.insert(e);

    for (auto e : fn->backgroundEntities)
        m_visibleEntities.insert(e);

    m_uiLayer = fn->uiLayer;
    m_objectLayer = fn->objectLayer;

    for (auto e : m_uiLayer)
        m_visibleEntities.insert(e);

    for (auto e : m_objectLayer)
        m_visibleEntities.insert(e);

    // Treat editor preview as "running" when FlowExecutor has an active node
    const bool previewRunning = (FlowExecutor::get().currentFlowNode() != INVALID_ENTITY);
    if (GameInstance::get().isRunning() || previewRunning) {
        int idx = FlowExecutor::get().currentEventIndex();
        if (idx < (int)fn->eventSequence.size()) {
            m_eventEntity = fn->eventSequence[idx];
            m_visibleEntities.insert(m_eventEntity);
        }
    } else {
        for (auto e : fn->eventSequence)
            m_visibleEntities.insert(e);
    }

    //std::cout << "[SceneManager] updateVisibleEntities: visible=" << m_visibleEntities.size()
    //          << " uiLayer=" << m_uiLayer.size() << " objectLayer=" << m_objectLayer.size()
    //          << " eventEntity=" << (unsigned)m_eventEntity << std::endl;
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
    // Ensure a current FlowNode exists for end-to-end validation
    if (m_currentFlowNode == INVALID_ENTITY) {
        Entity meta = ProjectManager::getProjectMetaEntity();
        if (auto base = EntityManager::get().getComponent(meta, ComponentType::ProjectMetadata)) {
            auto pm = std::static_pointer_cast<ProjectMetaComponent>(base);
            if (pm && !pm->sceneNodes.empty()) {
                setCurrentFlowNode(pm->sceneNodes.front());
                std::cout << "[SceneManager] Auto-selected default FlowNode for editor preview: "
                          << (unsigned)pm->sceneNodes.front() << std::endl;
            }
        }
    }
    updateVisibleEntities(); // ensure latest visibility while previewing
    for (Entity e : getVisibleEntities()) {
        RenderSystem::renderEntityEditor(e);
    }
}

void SceneManager::renderRuntimeScene() {
    // Ensure a current FlowNode exists for end-to-end validation
    if (m_currentFlowNode == INVALID_ENTITY) {
        Entity meta = ProjectManager::getProjectMetaEntity();
        if (auto base = EntityManager::get().getComponent(meta, ComponentType::ProjectMetadata)) {
            auto pm = std::static_pointer_cast<ProjectMetaComponent>(base);
            if (pm && !pm->sceneNodes.empty()) {
                setCurrentFlowNode(pm->sceneNodes.front());
                std::cout << "[SceneManager] Auto-selected default FlowNode for runtime preview: "
                          << (unsigned)pm->sceneNodes.front() << std::endl;
            }
        }
    }
    updateVisibleEntities(); // ensure latest visibility while playing

    // Draw backgrounds first (explicitly), then 3D objects, event UI, and finally the UI layer
    if (auto node = EntityManager::get().getComponent<FlowNodeComponent>(m_currentFlowNode)) {
        for (Entity e : node->backgroundEntities) {
            if (e != INVALID_ENTITY) RenderSystem::renderEntityRuntime(e);
        }
    }

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

// Additions: render region setters/getters
void SceneManager::setRenderRegion(float x, float y, float w, float h) {
    m_renderRegionX = x;
    m_renderRegionY = y;
    m_renderRegionW = w;
    m_renderRegionH = h;
}

float SceneManager::getRenderRegionX() const { return m_renderRegionX; }
float SceneManager::getRenderRegionY() const { return m_renderRegionY; }
float SceneManager::getRenderRegionW() const { return m_renderRegionW; }
float SceneManager::getRenderRegionH() const { return m_renderRegionH; }
