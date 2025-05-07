#include "EngineManager.h"
#include <iostream>

EngineManager::EngineManager() {
    m_currentEngine = new TrpgEngine();
}

EngineManager::~EngineManager() {
    delete m_currentEngine;
}

bool EngineManager::initializeEngine() {
    return m_currentEngine->initializeEngine();
}

void EngineManager::tick() {
    // Simulation placeholder
    m_currentEngine->updateFrame(0.016f);  // Approx. 60 FPS
}

CEngine* EngineManager::getEngine() {
    return m_currentEngine;
}
