#pragma once

#include "../Core/CEngine.h"

class TrpgEngine : public CEngine {
public:
    TrpgEngine();
    ~TrpgEngine();

    bool initializeEngine() override;
    void updateFrame(float deltaTime) override;

private:
    bool m_initialized;
};
