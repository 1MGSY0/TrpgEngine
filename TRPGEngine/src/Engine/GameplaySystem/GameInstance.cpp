#include "GameInstance.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"
#include "Engine/RenderSystem/SceneManager.hpp"
#include "FlowExecutor.hpp"

GameInstance& GameInstance::get() {
    static GameInstance instance;
    return instance;
}

void GameInstance::startGame() {
    auto& em = EntityManager::get();

    for (auto e : em.getAllEntities()) {
        auto proj = em.getComponent<ProjectMetaComponent>(e);
        if (proj && proj->startNode != INVALID_ENTITY) {
            FlowExecutor::get().reset();
            SceneManager::get().setCurrentFlowNode(proj->startNode);
            GameInstance::get().reset();
            m_running = true;
            return;
        }
    }
}

void GameInstance::update(float deltaTime) {
    if (!m_running) return;
    FlowExecutor::get().tick();
}

void GameInstance::reset() {
    m_running = false;
    FlowExecutor::get().reset();
}
