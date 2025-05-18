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

void BuildSystem::copyAssets(const std::string& from, const std::string& to) {
    try {
        if (!fs::exists(from)) {
            std::cerr << "[BuildSystem] Source asset folder not found: " << from << "\n";
            return;
        }

        fs::create_directories(to);

        for (auto& entry : fs::recursive_directory_iterator(from)) {
            const auto& path = entry.path();
            auto relativePath = fs::relative(path, from);
            fs::copy(path, fs::path(to) / relativePath, fs::copy_options::overwrite_existing | fs::copy_options::recursive);
        }
    } catch (const std::exception& e) {
        std::cerr << "[BuildSystem] Asset copy failed: " << e.what() << "\n";
    }
}

void BuildSystem::copyRuntime(const std::string& to) {
    try {
        fs::path runtimeDir = "Runtime"; // assumed runtime folder in project
        if (!fs::exists(runtimeDir)) {
            std::cerr << "[BuildSystem] Runtime directory missing.\n";
            return;
        }

        fs::create_directories(to + "/Runtime");
        fs::copy(runtimeDir, to + "/Runtime", fs::copy_options::overwrite_existing | fs::copy_options::recursive);
    } catch (const std::exception& e) {
        std::cerr << "[BuildSystem] Runtime copy failed: " << e.what() << "\n";
    }
}