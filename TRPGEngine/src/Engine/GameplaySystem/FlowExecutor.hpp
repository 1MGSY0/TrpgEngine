#pragma once
#include "Engine/EntitySystem/Entity.hpp"
#include <string>

class FlowExecutor {
public:
    static FlowExecutor& get();

    void reset();
    void tick(); // Called each frame while game is running

    // Accessors
    Entity currentFlowNode() const;
    int currentEventIndex() const;
    Entity currentEventEntity() const;
    bool eventCompleted() const;

private:
    Entity m_activeFlowNode = INVALID_ENTITY;
    int m_currentEventIndex = 0;
    Entity m_lastEvent = INVALID_ENTITY;
    bool m_eventCompleted = false;

    void advanceEvent(); // Handles moving to next event or flow node

    bool runEvent(Entity event);
    bool handleDialogue(Entity);
    bool handleUIButton(Entity);

    Entity findFlowNodeByName(const std::string& name);
};
