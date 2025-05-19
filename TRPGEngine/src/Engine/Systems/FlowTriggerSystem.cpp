#include "FlowTriggerSystem.hpp"

FlowTriggerSystem& FlowTriggerSystem::get() {
    static FlowTriggerSystem inst;
    return inst;
}

void FlowTriggerSystem::renderAndUpdate() {
    auto& em = EntityManager::get();

    // On first call, find start node
    if (m_currentNode == INVALID_ENTITY) {
        // find first FlowNodeComponent with isStart==true
        for (auto e : em.getAllEntities()) {
            auto fn = em.getComponent<FlowNodeComponent>(e);
            if (fn && fn->isStart) {
                m_currentNode = e;
                break;
            }
        }
    }

    if (m_currentNode == INVALID_ENTITY) return;

    auto fn = em.getComponent<FlowNodeComponent>(m_currentNode);
    ImGui::Begin("Flow");

    // Display dialogues
    for (auto child : fn->dialogueEntities) {
        auto dlg = em.getComponent<DialogueComponent>(child);
        for (auto& line : dlg->lines) {
            ImGui::TextWrapped("%s", line.c_str());
        }
    }

    // If waiting for continue click, show Continue
    if (m_waitingForClick) {
        if (ImGui::Button("Continue")) {
            m_waitingForClick = false;
            // Move to nextNode stored
            if (!fn->nextNode.empty()) {
                m_currentNode = fn->nextNode;
            }
        }
        ImGui::End();
        return;
    }

    // Show option buttons
    for (auto& opt : fn->options) {
        if (ImGui::Button(opt.label.c_str())) {
            // Roll dice
            m_lastDice = rand() % 20 + 1;
            bool success = m_lastDice >= opt.difficulty;
            fn->nextNode = success ? opt.nextOnSuccess : opt.nextOnFailure;
            m_waitingForClick = true;
        }
    }

    ImGui::End();
}
