#pragma once

class CEngine {
public:
    virtual ~CEngine() {}
    virtual bool initializeEngine() = 0;
    virtual void updateFrame(float deltaTime) = 0;
};
