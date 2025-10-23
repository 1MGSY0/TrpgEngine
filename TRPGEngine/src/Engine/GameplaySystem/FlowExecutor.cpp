#include "FlowExecutor.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/DialogueComponent.hpp"
#include "Engine/EntitySystem/Components/UIButtonComponent.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/RenderSystem/SceneManager.hpp"
// + include Choice/Dice to detect interactive events
#include "Engine/EntitySystem/Components/ChoiceComponent.hpp"
#include "Engine/EntitySystem/Components/DiceRollComponent.hpp"
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"
#include "Project/ProjectManager.hpp"

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
    // + In editor, default to the SceneManager-selected node if none is active
    if (m_activeFlowNode == INVALID_ENTITY) {
        m_activeFlowNode = SceneManager::get().getCurrentFlowNode();
    }
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

    // Do NOT auto-complete interactive events (let HUD drive them)
    if (em.hasComponent(event, ComponentType::Choice) ||
        em.hasComponent(event, ComponentType::DiceRoll)) {
        return false; // stay on current event until HUD interaction routes/advances
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

        // If target set, first try in-scene @Event: jump, else scene by name
        if (!comp->targetFlowNode.empty()) {
            // In-scene jump: @Event:ID
            const std::string tag = "@Event:";
            if (comp->targetFlowNode.rfind(tag, 0) == 0) {
                try {
                    Entity jumpTo = (Entity)std::stoull(comp->targetFlowNode.substr(tag.size()));
                    // find index in current node's eventSequence
                    auto node = em.getComponent<FlowNodeComponent>(m_activeFlowNode);
                    if (node) {
                        for (int i = 0; i < (int)node->eventSequence.size(); ++i) {
                            if (node->eventSequence[i] == jumpTo) {
                                // Jump to that event index (no advance increment)
                                m_currentEventIndex = i;
                                m_lastEvent = INVALID_ENTITY; // force re-eval next tick
                                return false;
                            }
                        }
                    }
                } catch (...) {
                    // fall back to default advance
                }
            } else {
                // Scene by name
                Entity next = findFlowNodeByName(comp->targetFlowNode);
                if (next != INVALID_ENTITY) {
                    SceneManager::get().setCurrentFlowNode(next);
                    reset();
                    return false; // Wait next frame
                }
            }
        }

        // Default: complete event (advance to next event/scene)
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

        // First try in-scene event jump via @Event:ID
        if (!comp->targetFlowNode.empty()) {
            const std::string tag = "@Event:";
            if (comp->targetFlowNode.rfind(tag, 0) == 0) {
                try {
                    Entity jumpTo = (Entity)std::stoull(comp->targetFlowNode.substr(tag.size()));
                    if (auto node = em.getComponent<FlowNodeComponent>(m_activeFlowNode)) {
                        for (int i = 0; i < (int)node->eventSequence.size(); ++i) {
                            if (node->eventSequence[i] == jumpTo) {
                                m_currentEventIndex = i;
                                m_lastEvent = INVALID_ENTITY; // force re-eval next tick
                                return false;
                            }
                        }
                    }
                } catch (...) {
                    // fall through to scene-name/default
                }
            } else {
                // Scene by name
                Entity next = findFlowNodeByName(comp->targetFlowNode);
                if (next != INVALID_ENTITY) {
                    SceneManager::get().setCurrentFlowNode(next);
                    reset();
                    return false;
                }
            }
        }

        // Default: complete this event (advance to next)
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
        // Prefer explicit Next Node
        Entity nextScene = (flow->nextNode != INVALID_ENTITY) ? (Entity)flow->nextNode : INVALID_ENTITY;

        // If no Next Node, fall back to next scene in ProjectMeta order
        if (nextScene == INVALID_ENTITY) {
            Entity metaEntity = ProjectManager::getProjectMetaEntity();
            auto metaBase = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
            if (metaBase) {
                auto meta = std::static_pointer_cast<ProjectMetaComponent>(metaBase);
                for (size_t i = 0; i < meta->sceneNodes.size(); ++i) {
                    if (meta->sceneNodes[i] == m_activeFlowNode) {
                        if (i + 1 < meta->sceneNodes.size()) {
                            nextScene = meta->sceneNodes[i + 1];
                        }
                        break;
                    }
                }
            }
        }

        if (nextScene != INVALID_ENTITY) {
            SceneManager::get().setCurrentFlowNode(nextScene);
            reset(); // bind to new scene on next tick
        } else {
            // End of flow
            m_activeFlowNode = INVALID_ENTITY;
            m_currentEventIndex = 0;
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