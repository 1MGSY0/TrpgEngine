#include "ProjectManager.hpp"
#include "Resources/ResourceManager.hpp"

#include <json.hpp>
#include <fstream>
#include <filesystem>
#include <ctime>
#include <iostream>

using json = nlohmann::json;
namespace fs = std::filesystem;

Entity ProjectManager::s_projectMetaEntity = INVALID_ENTITY;

std::string ProjectManager::s_currentProjectPath = "";
std::string ProjectManager::s_tempLoadPath = "";

static std::string ensureTrpgExtension(const std::string& path) {
    fs::path p(path);
    return (p.extension() != ".trpgproj") ? p.string() + ".trpgproj" : p.string();
}

bool ProjectManager::CreateNewProject(const std::string& projectName, const std::string& projectPath) {
    if (projectName.empty() || projectPath.empty()) {
        std::cerr << "[ProjectManager] Invalid project name or path.\n";
        return false;
    }

    fs::path fullPath = ensureTrpgExtension(projectPath);
    if (fs::exists(fullPath)) {
        std::cerr << "[ProjectManager] Project already exists at: " << fullPath << "\n";
        return false;
    }

    // Create project meta entity
    s_projectMetaEntity = EntityManager::get().createEntity();
    EntityManager::get().setEntityMeta(s_projectMetaEntity, projectName, EntityType::ProjectMeta);
    setProjectMetaEntity(s_projectMetaEntity);

    ResourceManager::get().setUnsavedChanges(true);
    return true;
}

bool ProjectManager::saveProjectToFile(const std::string& filePath) {
    if (s_projectMetaEntity == INVALID_ENTITY) {
        std::cerr << "[ProjectManager] No project meta entity to save.\n";
        return false;
    }

    nlohmann::json projectJson = EntityManager::get().serializeEntity(s_projectMetaEntity);

    std::ofstream out(filePath);
    if (!out.is_open()) {
        std::cerr << "[ProjectManager] Failed to write project file: " << filePath << "\n";
        return false;
    }

    out << projectJson.dump(4);
    std::cout << "[ProjectManager] Project saved to " << filePath << "\n";

    ResourceManager::get().setUnsavedChanges(false);
    return true;
}

bool ProjectManager::save() {
    if (s_currentProjectPath.empty()) return false;
    return saveProjectToFile(s_currentProjectPath);
}

bool ProjectManager::loadProject(const std::string& filePath) {
    std::ifstream in(filePath);
    if (!in.is_open()) {
        std::cerr << "[ProjectManager] Failed to open project file: " << filePath << "\n";
        return false;
    }

    nlohmann::json rootJson;
    in >> rootJson;

    EntityManager::get().clear();  // reset scene
    s_projectMetaEntity = EntityManager::get().deserializeEntity(rootJson);

    std::cout << "[ProjectManager] Project loaded from " << filePath << "\n";
    setCurrentProjectPath(filePath);
    ResourceManager::get().setUnsavedChanges(false);
    return true;
}

std::string ProjectManager::getCurrentProjectPath() {
    return s_currentProjectPath;
}

void ProjectManager::setCurrentProjectPath(const std::string& path) {
    s_currentProjectPath = path;
}

std::string ProjectManager::getTempLoadPath() {
    return s_tempLoadPath;
}

void ProjectManager::setTempLoadPath(const std::string& path) {
    s_tempLoadPath = path;
}
