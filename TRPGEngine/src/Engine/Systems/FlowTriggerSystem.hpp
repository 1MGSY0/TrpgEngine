#pragma once
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/EntitySystem/Components/DialogueComponent.hpp"
#include <imgui.h>

class FlowTriggerSystem {
public:
    static FlowTriggerSystem& get();

    // Called each frame to render flow UI and handle triggers
    void renderAndUpdate();

private:
    FlowTriggerSystem() = default;

    Entity m_currentNode = INVALID_ENTITY;
    bool m_waitingForClick = false;
    int  m_lastDice = 0;
};