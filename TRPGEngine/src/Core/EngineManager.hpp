#pragma once
#include <string>
#include "Engine/EntitySystem/Entity.hpp"


class EngineManager {
public:
    static EngineManager& get();

    void initialize();  // Initializes the engine runtime
    bool buildGame(const std::string& outputDirectory);  // Exports final game

private:
    EngineManager() = default;
};