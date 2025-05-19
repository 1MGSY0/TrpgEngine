#pragma once

#include "Engine/EntitySystem/EntityManager.hpp"
#include <string>

class SceneRuntime {
public:
    static SceneRuntime& get();

    void loadScene(const std::string& path);
    void update();
    void render();
    void reset();

    bool isSceneLoaded() const;

private:
    SceneRuntime() = default;
    std::string m_currentScenePath;
    bool m_sceneLoaded = false;
};
