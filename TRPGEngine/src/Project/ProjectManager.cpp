#include "ProjectManager.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Assets/AssetRegistry.h"
#include "Engine/Assets/AssetType.h"
#include "Engine/Assets/AssetBase.h"

#include <json.hpp>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

std::string ProjectManager::s_currentProjectPath = "";
std::string ProjectManager::s_tempLoadPath = "";

static std::string ensureTrpgExtension(const std::string& path) {
    fs::path p(path);
    return (p.extension() != ".trpgproj") ? p.string() + ".trpgproj" : p.string();
}

bool ProjectManager::saveProjectToFile(const std::string& filePath) {
    json j;
    auto& rm = ResourceManager::get();

    for (const auto& info : AssetTypeRegistry::getAllTypes()) {
        auto assets = rm.getAssets(info.type);
        if (!assets.empty()) {
            json arr;
            for (const auto& asset : assets)
                arr.push_back(asset->toJson());
            j[info.key] = arr;
        }
    }

    j["meta"] = {
        {"name", fs::path(filePath).stem().string()},
        {"saved_at", time(nullptr)},
        {"version", "0.1"}
    };

    std::ofstream file(filePath);
    if (!file.is_open()) return false;
    file << j.dump(4);
    return true;
}

bool ProjectManager::save() {
    if (s_currentProjectPath.empty()) return false;
    return saveProjectToFile(s_currentProjectPath);
}

bool ProjectManager::loadProject(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    json j;
    file >> j;

    auto& rm = ResourceManager::get();
    rm.clear();

    for (const auto& info : AssetTypeRegistry::getAllTypes()) {
        const std::string& key = info.key;
        if (j.contains(key)) {
            AssetRegistry::loadFromJsonArray(info.type, j[key]);
        }
    }

    setCurrentProjectPath(filePath);
    rm.setUnsavedChanges(false);
    return true;
}

void ProjectManager::setCurrentProjectPath(const std::string& filePath) {
    s_currentProjectPath = ensureTrpgExtension(filePath);
}

std::string ProjectManager::getCurrentProjectPath() {
    return s_currentProjectPath;
}

void ProjectManager::setTempLoadPath(const std::string& filePath) {
    s_tempLoadPath = ensureTrpgExtension(filePath);
}

std::string ProjectManager::getTempLoadPath() {
    return s_tempLoadPath;
}
