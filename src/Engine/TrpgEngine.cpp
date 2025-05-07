#include "TrpgEngine.h"
#include <iostream>

TrpgEngine::TrpgEngine() {
    m_initialized = false;
}

TrpgEngine::~TrpgEngine() {}

bool TrpgEngine::initializeEngine() {
    std::cout << "TRPG Engine initializing subsystems...\n";
    m_initialized = true;
    return m_initialized;
}

void TrpgEngine::updateFrame(float deltaTime) {
    // Later you’ll update systems here (GameLogicSystem, UI, etc.)
    if (!m_initialized) return;
    std::cout << "Updating frame: " << deltaTime << "s\n";
}
