#include "ProjectManager.hpp"
#include "Resources/ResourceManager.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"

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
    json projectMeta;
    projectMeta["meta"] = {
        {"name", fs::path(filePath).stem().string()},
        {"saved_at", time(nullptr)},
        {"version", "0.2"}
    };

    fs::path scenesFolder = fs::path("Assets/Scenes");
    fs::create_directories(scenesFolder);

    for (auto entity : EntityManager::get().getAllEntities()) {
        json j = EntityManager::get().serializeEntity(entity);
        std::string name = "entity_" + std::to_string(entity) + ".entity";
        std::ofstream out(scenesFolder / name);
        if (out.is_open()) out << j.dump(4);
    }

    std::ofstream out(filePath);
    if (!out.is_open()) return false;
    out << projectMeta.dump(4);
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

    EntityManager::get().clear();

    fs::path scenesFolder = fs::path("Assets/Scenes");
    if (fs::exists(scenesFolder)) {
        EntityManager::get().loadEntitiesFromFolder(scenesFolder.string());
    }

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
