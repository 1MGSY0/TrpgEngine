#pragma once
#include <string>
#include "Engine/EntitySystem/Entity.hpp"

extern Entity m_projectMetaEntity;  // <- only declare here

class EngineManager {
public:
    static EngineManager& get();

    Entity getProjectMetaEntity() const { return m_projectMetaEntity; }

    void initialize();  // Initializes the engine runtime
    bool buildGame(const std::string& outputDirectory);  // Exports final game

private:
    EngineManager() = default;
};