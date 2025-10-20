#include "DataLoader.h"
#include <fstream>
#include <json.hpp>

using json = nlohmann::json;

bool DataLoader::load(const std::string& path, GameData& outData) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    outData = GameData{}; // reset
    json j;
    file >> j;

    // Characters (new schema)
    if (j.contains("characters") && j["characters"].is_array()) {
        for (const auto& c : j["characters"]) {
            GameData::Character ch;
            if (c.contains("name")) ch.name = c["name"].get<std::string>();
            if (c.contains("portrait")) ch.portrait = c["portrait"].get<std::string>();
            if (c.contains("stats") && c["stats"].is_object()) {
                for (auto it = c["stats"].begin(); it != c["stats"].end(); ++it) {
                    if (it.value().is_number_integer())
                        ch.stats[it.key()] = it.value().get<int>();
                }
            }
            outData.characters.push_back(std::move(ch));
        }
    } else if (j.contains("characters")) {
        // Legacy minimal schema: array or objects with name only
        for (const auto& c : j["characters"]) {
            GameData::Character ch;
            if (c.contains("name")) ch.name = c["name"].get<std::string>();
            else if (c.is_string()) ch.name = c.get<std::string>();
            outData.characters.push_back(std::move(ch));
        }
    }

    // Flow graph (new schema)
    if (j.contains("startNode") && j["startNode"].is_number_integer()) {
        outData.startNodeId = j["startNode"].get<int>();
    }
    if (j.contains("flows") && j["flows"].is_array()) {
        for (const auto& n : j["flows"]) {
            GameData::FlowNode fn;
            if (n.contains("id")) fn.id = n["id"].get<int>();
            if (n.contains("type")) fn.type = n["type"].get<std::string>();
            if (n.contains("text")) fn.text = n["text"].get<std::string>();
            if (n.contains("speaker")) fn.speaker = n["speaker"].get<std::string>();
            if (n.contains("next") && n["next"].is_number_integer()) fn.next = n["next"].get<int>();
            if (n.contains("stat")) fn.stat = n["stat"].get<std::string>();
            if (n.contains("threshold") && n["threshold"].is_number_integer()) fn.threshold = n["threshold"].get<int>();
            if (n.contains("successNext") && n["successNext"].is_number_integer()) fn.successNext = n["successNext"].get<int>();
            if (n.contains("failNext") && n["failNext"].is_number_integer()) fn.failNext = n["failNext"].get<int>();
            if (n.contains("choices") && n["choices"].is_array()) {
                for (const auto& ch : n["choices"]) {
                    GameData::FlowChoice fc;
                    if (ch.contains("text")) fc.text = ch["text"].get<std::string>();
                    if (ch.contains("next") && ch["next"].is_number_integer()) fc.next = ch["next"].get<int>();
                    fn.choices.push_back(std::move(fc));
                }
            }
            outData.flow.push_back(std::move(fn));
        }
    }

    // Legacy content (fallback)
    if (outData.flow.empty() && j.contains("texts") && j["texts"].is_array()) {
        for (const auto& t : j["texts"]) {
            if (t.contains("text")) outData.texts.push_back(t["text"].get<std::string>());
            else if (t.is_string()) outData.texts.push_back(t.get<std::string>());
        }
    }
    if (j.contains("audio") && j["audio"].is_array()) {
        for (const auto& a : j["audio"]) {
            if (a.contains("filename")) outData.audios.push_back(a["filename"].get<std::string>());
            else if (a.is_string()) outData.audios.push_back(a.get<std::string>());
        }
    }

    return true;
}
