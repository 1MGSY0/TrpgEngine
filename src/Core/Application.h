#pragma once

#include "../Engine/TrpgEngine.h"
#include "../UI/EditorUI.h"
#include "../Engine/EngineManager.h"

class Application {
public:
    Application();
    ~Application();

    bool initEngine();
    void run();
    void quit();

private:
    EngineManager* m_engineManager;
    EditorUI* m_editorUI;

    bool m_running;
};
