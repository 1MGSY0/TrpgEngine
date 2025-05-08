#include "DataLoader.h"
#include <fstream>
#include <json.hpp>

using json = nlohmann::json;

bool DataLoader::load(const std::string& path, GameData& outData) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    json j;
    file >> j;

    for (const auto& t : j["texts"]) {
        outData.texts.push_back(t["text"].get<std::string>());
    }

    for (const auto& c : j["characters"]) {
        GameData::Character character;
        character.name = c["name"].get<std::string>();
        outData.characters.push_back(character);
    }

    for (const auto& a : j["audio"]) {
        outData.audios.push_back(a["filename"].get<std::string>());
    }

    return true;
}
