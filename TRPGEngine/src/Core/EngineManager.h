#pragma once
#include <string>

class EngineManager {
public:
    static EngineManager& get();

    // Initializes the engine runtime (if needed)
    void initialize();

    // Exports the final game (TRPG runtime + assets)
    bool buildGame(const std::string& outputDirectory);

private:
    EngineManager() = default;
};
