#include "FlowExecutor.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/DialogueComponent.hpp"
#include "Engine/EntitySystem/Components/UIButtonComponent.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/RenderSystem/SceneManager.hpp"

FlowExecutor& FlowExecutor::get() {
    static FlowExecutor inst;
    return inst;
}

void FlowExecutor::reset() {
    m_activeFlowNode = INVALID_ENTITY;
    m_currentEventIndex = 0;
    m_lastEvent = INVALID_ENTITY;
    m_eventCompleted = false;
}

void FlowExecutor::tick() {
    if (m_activeFlowNode == INVALID_ENTITY) return;

    auto& em = EntityManager::get();
    auto flow = em.getComponent<FlowNodeComponent>(m_activeFlowNode);
    if (!flow || m_currentEventIndex >= flow->eventSequence.size()) return;

    Entity currentEvent = flow->eventSequence[m_currentEventIndex];
    bool finished = runEvent(currentEvent);

    if (finished) {
        advanceEvent();
    }
}

bool FlowExecutor::runEvent(Entity event) {
    if (event != m_lastEvent) {
        m_lastEvent = event;
        m_eventCompleted = false;
    }

    if (m_eventCompleted) return true;

    auto& em = EntityManager::get();

    if (em.hasComponent(event, ComponentType::Dialogue)) {
        return handleDialogue(event);
    }

    if (em.hasComponent(event, ComponentType::UIButton)) {
        return handleUIButton(event);
    }

    // Default: complete unknown events immediately
    m_eventCompleted = true;
    return true;
}

bool FlowExecutor::handleDialogue(Entity entity) {
    auto& em = EntityManager::get();
    auto comp = em.getComponent<DialogueComponent>(entity);
    if (!comp) return true;

    if (comp->triggered) {
        comp->triggered = false;
        if (!comp->targetFlowNode.empty()) {
            Entity next = findFlowNodeByName(comp->targetFlowNode);
            if (next != INVALID_ENTITY) {
                SceneManager::get().setCurrentFlowNode(next);
                reset();
                return false; // Wait next frame
            }
        }
        m_eventCompleted = true;
        return true;
    }

    return false;
}

bool FlowExecutor::handleUIButton(Entity entity) {
    auto& em = EntityManager::get();
    auto comp = em.getComponent<UIButtonComponent>(entity);
    if (!comp) return true;

    if (comp->triggered) {
        comp->triggered = false;
        if (!comp->targetFlowNode.empty()) {
            Entity next = findFlowNodeByName(comp->targetFlowNode);
            if (next != INVALID_ENTITY) {
                SceneManager::get().setCurrentFlowNode(next);
                reset();
                return false;
            }
        }
        m_eventCompleted = true;
        return true;
    }

    return false;
}

void FlowExecutor::advanceEvent() {
    auto& em = EntityManager::get();
    auto flow = em.getComponent<FlowNodeComponent>(m_activeFlowNode);
    if (!flow) return;

    m_currentEventIndex++;

    if (m_currentEventIndex >= flow->eventSequence.size()) {
        if (flow->nextNode != INVALID_ENTITY) {
            SceneManager::get().setCurrentFlowNode(flow->nextNode);
            reset();
        } else {
            m_activeFlowNode = INVALID_ENTITY; // End
        }
    }
}

Entity FlowExecutor::findFlowNodeByName(const std::string& name) {
    auto& em = EntityManager::get();
    for (Entity e : em.getAllEntities()) {
        auto node = em.getComponent<FlowNodeComponent>(e);
        if (node && node->name == name)
            return e;
    }
    return INVALID_ENTITY;
}


Entity FlowExecutor::currentFlowNode() const {
    return m_activeFlowNode;
}

int FlowExecutor::currentEventIndex() const {
    return m_currentEventIndex;
}

Entity FlowExecutor::currentEventEntity() const {
    return m_lastEvent;
}

bool FlowExecutor::eventCompleted() const {
    return m_eventCompleted;
}