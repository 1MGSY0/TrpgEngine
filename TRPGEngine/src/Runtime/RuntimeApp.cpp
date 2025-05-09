#include "RuntimeApp.h"
#include "DataLoader.h"
#include <iostream>

void RuntimeApp::run(const std::string& dataFilePath) {
    std::cout << "[TRPG Runtime] Launching game...\n";

    GameData data;
    if (!DataLoader::load(dataFilePath, data)) {
        std::cerr << "[Runtime] Failed to load data.\n";
        return;
    }

    // Simulate loop
    std::cout << "Project loaded.\nCharacters:\n";
    for (const auto& c : data.characters) {
        std::cout << "- " << c.name << "\n";
    }

    std::cout << "Narrative begins...\n";
    for (const auto& t : data.texts) {
        std::cout << t << "\n";
    }

    std::cout << "[TRPG Runtime] Game finished.\n";
}
