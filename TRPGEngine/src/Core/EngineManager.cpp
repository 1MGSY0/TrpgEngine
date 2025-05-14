#include "EngineManager.hpp"
#include "Project/BuildSystem.hpp"
#include "Project/ProjectManager.hpp"
#include <iostream>

EngineManager& EngineManager::get() {
    static EngineManager instance;
    return instance;
}

void EngineManager::initialize() {
    // TODO: Initialize runtime systems, subsystems etc.
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
