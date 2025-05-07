#include "ProjectManager.h"
#include "Project/ResourceManager.h"
#include "Assets/TextAsset.h"
#include "Assets/Character.h"
#include "Assets/AudioAsset.h"
#include <json.hpp>
#include <fstream>

using json = nlohmann::json;

bool ProjectManager::saveProject(const std::string& directory) {
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
        {"name", "Untitled Project"},
        {"saved_at", time(nullptr)},
        {"version", "0.1"}
    };

    std::filesystem::create_directories(directory);
    std::ofstream file(directory + "/project.trpgproj");
    if (!file.is_open()) return false;

    file << j.dump(4);
    file.close();

    setCurrentProjectPath(directory);
    return true;
}

bool ProjectManager::loadProject(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    json j;
    file >> j;
    file.close();

    auto& rm = ResourceManager::get();

    // Clear previous project (optional)
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

    setCurrentProjectPath(std::filesystem::path(filePath).parent_path().string());
    return true;
}


std::string ProjectManager::s_currentProjectPath = "";

void ProjectManager::setCurrentProjectPath(const std::string& path) {
    s_currentProjectPath = path;
}
std::string ProjectManager::getCurrentProjectPath() {
    return s_currentProjectPath;
}