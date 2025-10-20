#include "RuntimeApp.h"
#include "DataLoader.h"
#include <iostream>
#include <random>
#include <limits>

static const GameData::FlowNode* findNode(const std::vector<GameData::FlowNode>& nodes, int id) {
    for (const auto& n : nodes) if (n.id == id) return &n;
    return nullptr;
}

void RuntimeApp::run(const std::string& dataFilePath) {
    std::cout << "[TRPG Runtime] Launching game...\n";

    GameData data;
    if (!DataLoader::load(dataFilePath, data)) {
        std::cerr << "[Runtime] Failed to load data.\n";
        return;
    }

    std::cout << "Project loaded.\nCharacters:\n";
    for (const auto& c : data.characters) {
        std::cout << "- " << c.name << "\n";
    }

    // If flow exists, run it; else fallback to legacy text listing.
    if (!data.flow.empty() && data.startNodeId != -1) {
        std::mt19937 rng(std::random_device{}());
        auto getStat = [&](const std::string& stat) -> int {
            if (data.characters.empty() || stat.empty()) return 0;
            auto it = data.characters.front().stats.find(stat);
            return (it != data.characters.front().stats.end()) ? it->second : 0;
        };

        int current = data.startNodeId;
        while (true) {
            auto node = findNode(data.flow, current);
            if (!node) {
                std::cout << "[Runtime] Missing node id=" << current << ". Ending.\n";
                break;
            }

            const std::string& type = node->type;
            if (type == "Start") {
                current = (node->next != -1) ? node->next : current; // usually Start -> next
                if (current == node->id) { std::cout << "[Runtime] Start has no next.\n"; break; }
                continue;
            } else if (type == "Dialogue") {
                if (!node->speaker.empty())
                    std::cout << node->speaker << ": ";
                std::cout << node->text << "\n";
                std::cout << "[Press Enter to continue]\n";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                current = (node->next != -1) ? node->next : -1;
                if (current == -1) break;
            } else if (type == "Narrative") {
                std::cout << node->text << "\n";
                std::cout << "[Press Enter to continue]\n";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                current = (node->next != -1) ? node->next : -1;
                if (current == -1) break;
            } else if (type == "Choice") {
                if (!node->text.empty())
                    std::cout << node->text << "\n";
                for (size_t i = 0; i < node->choices.size(); ++i) {
                    std::cout << (i + 1) << ") " << node->choices[i].text << "\n";
                }
                std::cout << "Select option: ";
                int sel = 0;
                std::cin >> sel;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                sel = std::max(1, std::min(sel, (int)node->choices.size()));
                current = node->choices[sel - 1].next;
                if (current == -1) break;
            } else if (type == "DiceCheck") {
                int statVal = getStat(node->stat.empty() ? "debate" : node->stat);
                int maxRoll = 10; // minimal d10
                std::uniform_int_distribution<int> dist(1, maxRoll);
                int roll = dist(rng);
                bool success = false;
                if (node->threshold >= 0) success = (roll <= node->threshold);
                else success = (roll <= statVal);

                std::cout << "[DiceCheck] Roll d" << maxRoll << " = " << roll
                          << " vs " << (node->threshold >= 0 ? node->threshold : statVal)
                          << " -> " << (success ? "SUCCESS" : "FAIL") << "\n";
                current = success ? node->successNext : node->failNext;
                if (current == -1) break;
            } else if (type == "End") {
                std::cout << "[TRPG Runtime] Game finished.\n";
                break;
            } else {
                std::cout << "[Runtime] Unknown node type: " << type << ". Ending.\n";
                break;
            }
        }
        return;
    }

    // Legacy fallback
    std::cout << "Narrative begins...\n";
    for (const auto& t : data.texts) {
        std::cout << t << "\n";
    }
    std::cout << "[TRPG Runtime] Game finished.\n";
}
