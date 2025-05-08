#include "BuildSystem.h"
#include "ProjectManager.h"
#include "ResourceManager.h"
#include "Assets/TextAsset.h"
#include "Assets/Character.h"
#include "Assets/AudioAsset.h"

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
    fs::path buildPath = fs::path(outputDirectory);

    // 1. Export data.json (texts, characters, audio)
    auto& rm = ResourceManager::get();
    json j;

    for (auto& text : rm.getTexts())
        j["texts"].push_back(text->toJson());

    for (auto& character : rm.getCharacters())
        j["characters"].push_back(character->toJson());

    for (auto& audio : rm.getAudios())
        j["audio"].push_back(audio->toJson());

    std::ofstream dataFile(buildPath / "data.json");
    if (!dataFile.is_open()) {
        std::cerr << "[BuildSystem] Failed to write data.json.\n";
        return false;
    }

    dataFile << j.dump(4);
    dataFile.close();
    std::cout << "[BuildSystem] Exported data.json.\n";

    // 2. Copy asset files (e.g., images, audio files)
    fs::path assetsSource = fs::path(projectPath) / "Assets";
    fs::path assetsDest = buildPath / "Assets";
    copyAssets(assetsSource.string(), assetsDest.string());

    // 3. Copy runtime executable
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
    const fs::path runtimeSource = "bin/TRPGRuntime.exe";  
    const fs::path runtimeDest = fs::path(to) / "TRPGGame.exe";

    try {
        fs::copy_file(runtimeSource, runtimeDest, fs::copy_options::overwrite_existing);
        std::cout << "[BuildSystem] Runtime copied.\n";
    } catch (const fs::filesystem_error& e) {
        std::cerr << "[BuildSystem] Failed to copy runtime: " << e.what() << "\n";
    }
}
