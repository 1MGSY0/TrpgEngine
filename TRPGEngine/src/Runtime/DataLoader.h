#pragma once
#include <vector>
#include <string>
#include <unordered_map>

struct GameData {
    // --- Characters ---
    struct Character {
        std::string name;
        std::unordered_map<std::string, int> stats; // e.g., { "debate": 6 }
        std::string portrait;                       // optional path
    };
    std::vector<Character> characters;

    // --- Flow graph ---
    struct FlowChoice { std::string text; int next = -1; };
    struct FlowNode {
        int id = -1;
        std::string type;       // Start, Narrative, Dialogue, Choice, DiceCheck, End
        std::string text;       // used for Dialogue/Narrative
        std::string speaker;    // for Dialogue
        int next = -1;          // generic next for Narrative/Dialogue
        std::string stat;       // e.g. "debate" for DiceCheck
        int threshold = -1;     // optional threshold; if -1, compare roll <= character stat
        int successNext = -1;   // for DiceCheck
        int failNext = -1;      // for DiceCheck
        std::vector<FlowChoice> choices; // for Choice
    };
    int startNodeId = -1;
    std::vector<FlowNode> flow;

    // --- Legacy fields (fallback) ---
    std::vector<std::string> texts;
    std::vector<std::string> audios;
};

class DataLoader {
public:
    static bool load(const std::string& path, GameData& outData);
};
