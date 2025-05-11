#include "BuildSystem.h"
#include "ProjectManager.h"
#include "Engine/Assets/AssetType.h" 
#include "Engine/Resources/ResourceManager.h"

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
    auto& rm = ResourceManager::get();
    json j;

    for (const auto& info : AssetTypeRegistry::getAllTypes()) {
        auto assets = rm.getAssets(info.type);
        if (!assets.empty()) {
            for (const auto& asset : assets)
                j[info.key].push_back(asset->toJson());
        }
    }

    std::ofstream dataFile(fs::path(outputDirectory) / "data.json");
    if (!dataFile.is_open()) {
        std::cerr << "[BuildSystem] Failed to write data.json.\n";
        return false;
    }

    dataFile << j.dump(4);
    std::cout << "[BuildSystem] Exported data.json.\n";

    copyAssets(projectPath + "/Assets", outputDirectory + "/Assets");
    copyRuntime(outputDirectory);

    std::cout << "[BuildSystem] Build completed successfully.\n";
    return true;
}

void BuildSystem::copyAssets(const std::string& from, const std::string& to) {
    fs::create_directories(to);

    if (!fs::exists(from)) {
        std::cerr << "[BuildSystem] No assets to copy from: " << from << "\n";
        return;
    }

    for (const auto& entry : fs::recursive_directory_iterator(from)) {
        if (fs::is_regular_file(entry)) {
            fs::path relativePath = fs::relative(entry.path(), from);
            fs::path targetPath = fs::path(to) / relativePath;
            fs::create_directories(targetPath.parent_path());

            try {
                fs::copy_file(entry.path(), targetPath, fs::copy_options::overwrite_existing);
                std::cout << "[BuildSystem] Copied asset: " << relativePath << "\n";
            } catch (const fs::filesystem_error& e) {
                std::cerr << "[BuildSystem] Failed to copy " << entry.path() << ": " << e.what() << "\n";
            }
        }
    }
}

void BuildSystem::copyRuntime(const std::string& to) {
    fs::path runtimeSource = fs::path("bin") / "TRPGRuntime.exe";
    fs::path runtimeDest = fs::path(to) / "TRPGGame.exe";

    try {
        fs::copy_file(runtimeSource, runtimeDest, fs::copy_options::overwrite_existing);
        std::cout << "[BuildSystem] Copied runtime executable.\n";
    } catch (const fs::filesystem_error& e) {
        std::cerr << "[BuildSystem] Failed to copy runtime: " << e.what() << "\n";
    }
}
