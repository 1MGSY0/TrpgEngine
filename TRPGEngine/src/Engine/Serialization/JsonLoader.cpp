#include "JsonLoader.h"
#include <json.hpp>
#include <fstream>
#include <iostream>
#include "Engine/Entity/Components/CharacterComponent.h"
#include "Engine/Entity/Components/ScriptComponent.h"

using json = nlohmann::json;

namespace JsonLoader {

    template<typename T>
    std::shared_ptr<T> loadJsonFile(const std::string& filePath) {
        std::ifstream in(filePath);
        if (!in.is_open()) {
            std::cerr << "[JsonLoader] Failed to open file: " << filePath << std::endl;
            return nullptr;
        }

        json j;
        in >> j;

        auto obj = std::make_shared<T>();
        return obj;
    }

//     // --- CharacterComponent Specialization ---
//     template<>
//     std::shared_ptr<CharacterComponent> loadJsonFile<CharacterComponent>(const std::string& filePath) {
//         std::ifstream in(filePath);
//         if (!in.is_open()) {
//             std::cerr << "[JsonLoader] Failed to open file: " << filePath << std::endl;
//             return nullptr;
//         }

//         json j;
//         in >> j;

//         auto character = std::make_shared<CharacterComponent>();
//         character->name = j.value("name", "");
//         character->iconImage = j.value("icon", "Assets/Icons/no_image.png");

//         if (j.contains("stats") && j["stats"].is_object()) {
//             for (auto& [key, val] : j["stats"].items()) {
//                 character->stats[key] = val.get<int>();
//             }
//         }

//         if (j.contains("states") && j["states"].is_object()) {
//             for (auto& [state, path] : j["states"].items()) {
//                 character->stateImages[state] = path.get<std::string>();
//             }
//         }

//         return character;
//     }

//     // --- ScriptComponent Specialization ---
//     template<>
//     std::shared_ptr<ScriptComponent> loadJsonFile<ScriptComponent>(const std::string& filePath) {
//         std::ifstream in(filePath);
//         if (!in.is_open()) {
//             std::cerr << "[JsonLoader] Failed to open file: " << filePath << std::endl;
//             return nullptr;
//         }

//         json j;
//         in >> j;

//         auto script = std::make_shared<ScriptComponent>();
//         script->name = j.value("name", "");
//         script->scriptPath = j.value("path", "");
//         return script;
//     }
} 