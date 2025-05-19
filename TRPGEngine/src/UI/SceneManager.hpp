#pragma once
#include <vector>
#include <string>
#include "UI/ScenePanel/ScenePanel.hpp"

class SceneManager {
public:
    static std::vector<std::string>& getSceneNames() {
        static std::vector<std::string> names = { "Scene 1" };
        return names;
    }

    static std::vector<ScenePanel>& getScenePanels() {
        static std::vector<ScenePanel> panels = { ScenePanel() };
        return panels;
    }

    static void addScene(const std::string& name) {
        getSceneNames().push_back(name);
        getScenePanels().emplace_back();
    }

    static int getNextSceneIndex() {
        static int index = 2;
        return index++;
    }

    static int getCurrentSceneIndex() {
        return getSceneNames().size() - 1;
    }
};