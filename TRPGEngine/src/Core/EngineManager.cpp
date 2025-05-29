#include "EngineManager.hpp"
#include "Project/BuildSystem.hpp"
#include "Project/ProjectManager.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"

#include <iostream>

Entity m_projectMetaEntity = INVALID_ENTITY;

EngineManager& EngineManager::get() {
    static EngineManager instance;
    return instance;
}

void EngineManager::initialize() {
    std::cout << "[EngineManager] Initializing...\n";

    // Initialize EntityManager & Project Meta
    auto& em = EntityManager::get();
    m_projectMetaEntity = em.createEntity();
    em.addComponent(m_projectMetaEntity, std::make_shared<ProjectMetaComponent>());

    // Initialize other systems as needed
    // e.g. LuaScriptSystem::get().init();
    // FlowTriggerSystem::get().init();

    std::cout << "[EngineManager] Initialized.\n";
}

bool EngineManager::buildGame(const std::string& outputDirectory) {
    const std::string& projectPath = ProjectManager::getCurrentProjectPath();
    if (projectPath.empty()) {
        std::cerr << "[EngineManager] No project path. Cannot build.\n";
        return false;
    }
    return BuildSystem::buildProject(projectPath, outputDirectory);
}
