#include "EngineManager.hpp"
#include "Project/BuildSystem.hpp"
#include "Project/ProjectManager.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/ComponentType.hpp"
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"
#include "Project/ProjectManager.hpp"

#include <iostream>

Entity m_projectMetaEntity = INVALID_ENTITY;

EngineManager& EngineManager::get() {
    static EngineManager instance;
    return instance;
}

void EngineManager::initialize() {
    std::cout << "[EngineManager] Initializing...\n";

    // Populate the known component types and their factories/inspectors
    ComponentTypeRegistry::registerBuiltins();
    std::cout << "[Init] Registered components: " << ComponentTypeRegistry::getAllInfos().size() << "\n";

    // Initialize EntityManager & Project Meta

    if (ProjectManager::getCurrentProjectPath().empty()) {
        std::cout << "[EngineManager] No project loaded. Creating temporary project...\n";

        std::string defaultProjectName = "Untitled";
        std::string defaultProjectPath = "Runtime/" + defaultProjectName + ".trpgproj";

        // Ensure directory exists
        std::filesystem::create_directories("Runtime");

        if (!ProjectManager::CreateNewProject(defaultProjectName, defaultProjectPath)) {
            std::cerr << "[EngineManager] Failed to create default project.\n";
        } else {
            ProjectManager::setCurrentProjectPath(defaultProjectPath);
            std::cout << "[EngineManager] Default project created.\n";
            ProjectManager::requestProjectInfoPrompt();
        }
    }
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
