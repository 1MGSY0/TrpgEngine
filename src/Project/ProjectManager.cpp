#include "ProjectManager.h"
#include "Project/ResourceManager.h"
#include "Assets/TextAsset.h"
#include "Assets/Character.h"
#include "Assets/AudioAsset.h"
#include <json.hpp>
#include <fstream>

using json = nlohmann::json;

bool ProjectManager::saveProject(const std::string& filePath) {
    json j;

    auto& rm = ResourceManager::get();

    for (auto& t : rm.getTexts())
        j["texts"].push_back(t->toJson());

    for (auto& c : rm.getCharacters())
        j["characters"].push_back(c->toJson());

    for (auto& a : rm.getAudios())
        j["audio"].push_back(a->toJson());

    std::ofstream file(filePath);
    if (!file.is_open()) return false;

    file << j.dump(4); // Pretty print
    file.close();
    return true;
}

bool ProjectManager::loadProject(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    json j;
    file >> j;
    file.close();

    auto& rm = ResourceManager::get();

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

    return true;
}
