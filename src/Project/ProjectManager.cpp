#include "ProjectManager.h"
#include "Project/ResourceManager.h"
#include "Assets/TextAsset.h"
#include "Assets/Character.h"
#include "Assets/AudioAsset.h"

#include <json.hpp>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;

std::string ProjectManager::s_currentProjectPath = "";
std::string ProjectManager::s_tempLoadPath = "";

static std::string ensureTrpgExtension(const std::string& path) {
    std::filesystem::path p(path);
    if (p.extension() != ".trpgproj") {
        return p.string() + ".trpgproj";
    }
    return p.string();
}

bool ProjectManager::saveProjectToFile(const std::string& filePath) {
    const std::string finalPath = ensureTrpgExtension(filePath);

    json j;
    auto& rm = ResourceManager::get();

    for (auto& t : rm.getTexts())
        j["texts"].push_back(t->toJson());
    for (auto& c : rm.getCharacters())
        j["characters"].push_back(c->toJson());
    for (auto& a : rm.getAudios())
        j["audio"].push_back(a->toJson());

    // Project metadata
    j["meta"] = {
        {"name", std::filesystem::path(finalPath).stem().string()},
        {"saved_at", time(nullptr)},
        {"version", "0.1"}
    };

    std::ofstream file(finalPath);
    if (!file.is_open()) return false;

    file << j.dump(4);
    file.close();

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
    file.close();

    auto& rm = ResourceManager::get();
    rm.clear();

    if (j.contains("texts")) {
        for (auto& t : j["texts"])
            rm.addText(TextAsset::fromJson(t));
    }

    if (j.contains("characters")) {
        for (auto& c : j["characters"])
            rm.addCharacter(Character::fromJson(c));
    }

    if (j.contains("audio")) {
        for (auto& a : j["audio"])
            rm.addAudio(AudioAsset::fromJson(a));
    }

    setCurrentProjectPath(filePath);
    ResourceManager::get().setUnsavedChanges(false);
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
