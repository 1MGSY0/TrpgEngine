#include "BuildSystem.hpp"
#include "ProjectManager.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Resources/ResourceManager.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

bool BuildSystem::buildProject(const std::string& projectPath, const std::string& outputDirectory) {
    std::cout << "[BuildSystem] Starting build...\n";

    if (!fs::exists(projectPath)) {
        std::cerr << "[BuildSystem] Invalid project path: " << projectPath << "\n";
        return false;
    }

    fs::create_directories(outputDirectory);

    fs::path sceneOut = fs::path(outputDirectory) / "Entities";
    fs::create_directories(sceneOut);

    for (auto entity : EntityManager::get().getAllEntities()) {
        json j = EntityManager::get().serializeEntity(entity);
        std::string fileName = "entity_" + std::to_string(entity) + ".entity";
        std::ofstream out(sceneOut / fileName);
        if (out.is_open()) {
            out << j.dump(4);
        }
    }

    copyAssets(projectPath + "/Assets", outputDirectory + "/Assets");
    copyRuntime(outputDirectory);

    std::cout << "[BuildSystem] Build completed successfully.\n";
    return true;
}
