#include "SceneRuntime.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/DialogueComponent.hpp"
#include "Engine/EntitySystem/Components/CharacterComponent.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/EntitySystem/Components/TransformComponent.hpp"
#include "Engine/EntitySystem/Components/ScriptComponent.hpp"

#include <imgui.h>

SceneRuntime& SceneRuntime::get() {
    static SceneRuntime instance;
    return instance;
}

void SceneRuntime::start(const std::string& sceneFile) {
    EntityManager::get().clear();
    EntityManager::get().loadFromFile(sceneFile);

    m_currentNode = findStartNode();
    m_started = true;
    m_waitingForClick = false;
    m_diceRollInProgress = false;
    m_diceResult = -1;
}

void SceneRuntime::stop() {
    m_started = false;
    m_currentNode = INVALID_ENTITY;
    m_waitingForClick = false;
    m_diceRollInProgress = false;
    m_diceResult = -1;
}

bool SceneRuntime::isRunning() const {
    return m_started;
}

void SceneRuntime::update() {
    if (!m_started || m_currentNode == INVALID_ENTITY) return;

    auto* flow = EntityManager::get().getComponent<FlowNodeComponent>(m_currentNode);
    if (!flow) return;

    ImGui::Begin("Runtime Dialogue");

    if (m_waitingForClick) {
        if (ImGui::Button("Continue")) {
            m_waitingForClick = false;
            if (!m_nextNode.empty()) {
                m_currentNode = EntityManager::get().findEntityById(m_nextNode);
                m_nextNode.clear();
            } else {
                stop();  // End
            }
        }
        ImGui::End();
        return;
    }

    // Render dialogues
    for (Entity child : EntityManager::get().getChildren(m_currentNode)) {
        auto dialogue = EntityManager::get().getComponent<DialogueComponent>(child);
        if (dialogue) {
            for (const auto& line : dialogue->lines) {
                ImGui::TextWrapped("%s", line.c_str());
            }
        }
    }

    // Render options
    for (const auto& option : flow->options) {
        if (ImGui::Button(option.label.c_str())) {
            m_diceRollInProgress = true;
            m_diceResult = rollDice(option.difficulty);
            m_lastOption = option;
        }
    }

    if (m_diceRollInProgress) {
        ImGui::Text("Rolling dice for: %s", m_lastOption.label.c_str());
        ImGui::Text("Result: %d (Need >= %d)", m_diceResult, m_lastOption.difficulty);
        if (m_diceResult >= m_lastOption.difficulty) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Success!");
            m_nextNode = m_lastOption.nextOnSuccess;
        } else {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Failed!");
            m_nextNode = m_lastOption.nextOnFailure;
        }

        m_diceRollInProgress = false;
        m_waitingForClick = true;
    }

    ImGui::End();
}

Entity SceneRuntime::findStartNode() {
    for (Entity e : EntityManager::get().getAllEntities()) {
        auto flow = EntityManager::get().getComponent<FlowNodeComponent>(e);
        if (flow && flow->isStart)
            return e;
    }
    return INVALID_ENTITY;
}

int SceneRuntime::rollDice(int difficulty) {
    return rand() % 20 + 1; // Roll d20
}
