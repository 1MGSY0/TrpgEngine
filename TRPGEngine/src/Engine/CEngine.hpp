#pragma once
#include <string>

class CEngine {
public:
    virtual ~CEngine() = default;

    virtual bool initializeEngine() = 0;
    virtual bool loadProjectData(const std::string& path) = 0;
    virtual bool saveProjectData(const std::string& path) = 0;

    virtual void startGame() = 0;
    virtual void pauseGame() = 0;
    virtual void stopGame() = 0;
    virtual void shutdown() = 0;
};