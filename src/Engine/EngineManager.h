#pragma once

#include "../Engine/TrpgEngine.h"

class EngineManager {
public:
    EngineManager();
    ~EngineManager();

    bool initializeEngine();
    void tick();

    CEngine* getEngine();

private:
    TrpgEngine* m_currentEngine;
};
