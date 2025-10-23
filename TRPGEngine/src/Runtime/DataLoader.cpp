#include "DataLoader.h"
#include <fstream>
#include <json.hpp>
#include <unordered_map> // + resolve names/first events

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

    // NEW: Flow graph from exported "scenes" schema
    if (j.contains("scenes") && j["scenes"].is_array()) {
        const auto& scenes = j["scenes"];

        // First pass: map scene name -> first event id (if any), and id->first event
        std::unordered_map<std::string, int> sceneNameToFirstEvent;
        std::unordered_map<uint64_t, int> sceneIdToFirstEvent;
        int startFirstEvent = -1;

        for (const auto& sc : scenes) {
            uint64_t sid = sc.value("id", 0);
            std::string sname = sc.value("name", std::string{});
            int firstEventId = -1;
            if (sc.contains("events") && sc["events"].is_array()) {
                const auto& evs = sc["events"];
                for (const auto& ev : evs) {
                    if (ev.contains("id") && ev["id"].is_number_unsigned()) {
                        firstEventId = (int)ev["id"].get<uint64_t>();
                        break;
                    }
                }
            }
            sceneNameToFirstEvent[sname] = firstEventId;
            sceneIdToFirstEvent[sid] = firstEventId;

            bool isStart = sc.value("isStart", false);
            if (isStart && firstEventId != -1) startFirstEvent = firstEventId;
        }

        // Helper to resolve "target" string to numeric event id
        auto resolveTargetToEvent = [&](const std::string& tgt)->int {
            if (tgt.empty()) return -1;
            const std::string tag = "@Event:";
            if (tgt.rfind(tag, 0) == 0) {
                try { return (int)std::stoull(tgt.substr(tag.size())); } catch (...) { return -1; }
            }
            auto it = sceneNameToFirstEvent.find(tgt);
            if (it != sceneNameToFirstEvent.end()) return it->second;
            return -1;
        };

        // Second pass: flatten events into GameData::flow
        for (const auto& sc : scenes) {
            uint64_t sid = sc.value("id", 0);
            std::string sname = sc.value("name", std::string{});
            int sceneNextNode = sc.value("nextNode", -1); // entity id or -1

            int sceneFirstEvent = sceneIdToFirstEvent[sid];

            if (!(sc.contains("events") && sc["events"].is_array())) continue;
            const auto& evs = sc["events"];

            // Build vector of event ids for default "next event" fallback
            std::vector<int> eventIds;
            eventIds.reserve(evs.size());
            for (const auto& ev : evs) {
                if (ev.contains("id") && ev["id"].is_number_unsigned())
                    eventIds.push_back((int)ev["id"].get<uint64_t>());
            }

            for (size_t idx = 0; idx < evs.size(); ++idx) {
                const auto& ev = evs[idx];
                if (!ev.contains("id") || !ev["id"].is_number_unsigned()) continue;

                GameData::FlowNode fn;
                fn.id = (int)ev["id"].get<uint64_t>();
                fn.type = ev.value("type", std::string("Unknown"));
                fn.next = -1; // default; may be filled below

                // Dialogue
                if (fn.type == "Dialogue") {
                    // flatten text (join lines or take first)
                    std::string flat;
                    if (ev.contains("lines") && ev["lines"].is_array() && !ev["lines"].empty()) {
                        flat = ev["lines"][0].get<std::string>();
                    }
                    fn.text = flat;
                    // resolve target
                    std::string target = ev.value("target", std::string{});
                    int resolved = resolveTargetToEvent(target);
                    if (resolved != -1) {
                        fn.next = resolved;
                    } else {
                        // default: next event in the same scene
                        if (idx + 1 < eventIds.size()) {
                            fn.next = eventIds[idx + 1];
                        } else if (sceneNextNode != -1) {
                            // last event in scene -> go to next scene's first event (if any)
                            auto itF = sceneIdToFirstEvent.find((uint64_t)sceneNextNode);
                            if (itF != sceneIdToFirstEvent.end()) fn.next = itF->second;
                        }
                    }
                }
                // Choice
                else if (fn.type == "Choice") {
                    if (ev.contains("options") && ev["options"].is_array()) {
                        for (const auto& opt : ev["options"]) {
                            if (!opt.is_string()) continue;
                            std::string text = opt.get<std::string>();
                            // parse "Text -> Target"
                            std::string baseText = text;
                            std::string tgt;
                            const std::string delim = " -> ";
                            size_t pos = baseText.rfind(delim);
                            if (pos != std::string::npos) {
                                tgt = baseText.substr(pos + delim.size());
                                baseText = baseText.substr(0, pos);
                            }
                            GameData::FlowChoice fc;
                            fc.text = baseText;
                            int resolved = resolveTargetToEvent(tgt);
                            if (resolved != -1) {
                                fc.next = resolved;
                            } else {
                                // default to next event in scene
                                fc.next = (idx + 1 < eventIds.size()) ? eventIds[idx + 1] : -1;
                            }
                            fn.choices.push_back(std::move(fc));
                        }
                    } else {
                        // no options: default next to next event
                        if (idx + 1 < eventIds.size()) fn.next = eventIds[idx + 1];
                    }
                }
                // DiceRoll
                else if (fn.type == "DiceRoll") {
                    if (ev.contains("threshold") && ev["threshold"].is_number_integer())
                        fn.threshold = ev["threshold"].get<int>();
                    // resolve success/failure targets
                    std::string sSucc = ev.value("onSuccess", std::string{});
                    std::string sFail = ev.value("onFailure", std::string{});
                    int succ = resolveTargetToEvent(sSucc);
                    int fail = resolveTargetToEvent(sFail);
                    fn.successNext = succ;
                    fn.failNext = fail;
                    // default: leave next = -1 (branching handled by runtime)
                }
                // Unknown: default to next event
                else {
                    if (idx + 1 < eventIds.size()) fn.next = eventIds[idx + 1];
                }

                outData.flow.push_back(std::move(fn));
            }
        }

        // Start node = first event id of start scene (if present)
        if (outData.startNodeId == 0 || outData.startNodeId == -1) {
            outData.startNodeId = (startFirstEvent != -1) ? startFirstEvent : outData.startNodeId;
        }
    }

    // Flow graph (legacy "new schema" fallback)
    if (outData.flow.empty() && j.contains("startNode") && j["startNode"].is_number_integer()) {
        outData.startNodeId = j["startNode"].get<int>();
    }
    if (outData.flow.empty() && j.contains("flows") && j["flows"].is_array()) {
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
