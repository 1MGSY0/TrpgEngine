#include "ProjectManager.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Entity/Components/TextComponent.h"
#include "Engine/Entity/Components/CharacterComponent.h"
#include "Engine/Entity/Components/AudioComponent.h"

#include <json.hpp>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

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
    json j;
    auto& rm = ResourceManager::get();

    // Save components
    for (auto& t : rm.getAllTexts())
        j["texts"].push_back(t->toJson());

    for (auto& c : rm.getAllCharacters())
        j["characters"].push_back(c->toJson());

    for (auto& a : rm.getAllAudio())
        j["audio"].push_back(a->toJson());

    j["meta"] = {
        {"name", fs::path(filePath).stem().string()},
        {"saved_at", time(nullptr)},
        {"version", "0.1"}
    };

    std::ofstream file(filePath);
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

    // --- Text
    if (j.contains("texts")) {
        for (auto& t : j["texts"]) {
            auto text = std::make_shared<TextComponent>();
            text->fromJson(t);
            rm.addText(text->getName(), text);
        }
    }

    // --- Characters
    if (j.contains("characters")) {
        for (auto& c : j["characters"]) {
            auto character = std::make_shared<CharacterComponent>();
            character->fromJson(c);
            rm.addCharacter(character->getName(), character);
        }
    }

    // --- Audio
    if (j.contains("audio")) {
        for (auto& a : j["audio"]) {
            auto audio = std::make_shared<AudioComponent>();
            audio->fromJson(a);
            rm.addAudio(audio->getName(), audio);
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
