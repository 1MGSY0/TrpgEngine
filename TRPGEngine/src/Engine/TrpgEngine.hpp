// #pragma once

// #include "CEngine.h"
// #include "UI/SceneManager.h"
// //#include "Subsystems/GraphicsEngine.h"
// //#include "Subsystems/AudioEngine.h"
// //#include "Subsystems/PhysicsEngine.h"
// //#include "Subsystems/AnimationEngine.h"
// #include "UI/EditorUI.h"
// //#include "Subsystems/GameLogicSystem.h"
// //#include "Subsystems/DialogueSystem.h"
// //#include "Subsystems/EventSystem.h"
// //#include "Subsystems/ReplaySystem.h"
// //#include "Subsystems/HUDSystem.h"
// #include "Resources/ResourceManager.h"
// #include "Gameplay/Scene.h"

// class TrpgEngine : public CEngine {
// public:
//     bool initializeEngine() override;
//     bool loadProjectData(const std::string& path) override;
//     bool saveProjectData(const std::string& path) override;

//     void startGame() override;
//     void pauseGame() override;
//     void stopGame() override;
//     void shutdown() override;

//     void updateFrame(float deltaTime) override;

//     Scene* loadScene(const std::string& sceneName) override;
//     void setCurrentScene(Scene* scene) override;

//     GraphicsEngine* getGraphicsEngine() override { return m_graphics.get(); }
//     AudioEngine* getAudioEngine() override { return m_audio.get(); }
//     PhysicsEngine* getPhysicsEngine() override { return m_physics.get(); }
//     AnimationEngine* getAnimationEngine() override { return m_animation.get(); }
//     UIManager* getUIManager() override { return m_ui.get(); }
//     GameLogicSystem* getGameLogicSystem() override { return m_logic.get(); }
//     DialogueSystem* getDialogueSystem() override { return m_dialogue.get(); }
//     EventSystem* getEventSystem() override { return m_event.get(); }
//     ReplaySystem* getReplaySystem() override { return m_replay.get(); }
//     HUDSystem* getHUDSystem() override { return m_hud.get(); }
//     ResourceManager* getResourceManager() override { return m_resourceManager.get(); }
//     SceneDirector* getSceneDirector() override { return m_sceneDirector.get(); }

// private:
//     std::unique_ptr<GraphicsEngine> m_graphics;
//     std::unique_ptr<AudioEngine> m_audio;
//     std::unique_ptr<PhysicsEngine> m_physics;
//     std::unique_ptr<AnimationEngine> m_animation;
//     std::unique_ptr<UIManager> m_ui;
//     std::unique_ptr<GameLogicSystem> m_logic;
//     std::unique_ptr<DialogueSystem> m_dialogue;
//     std::unique_ptr<EventSystem> m_event;
//     std::unique_ptr<ReplaySystem> m_replay;
//     std::unique_ptr<HUDSystem> m_hud;
//     std::unique_ptr<ResourceManager> m_resourceManager;
//     std::unique_ptr<SceneDirector> m_sceneDirector;

//     Scene* m_currentScene = nullptr;
// };