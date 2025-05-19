// #include "TrpgEngine.h"
// #include <iostream>

// TrpgEngine::TrpgEngine() {}

// TrpgEngine::~TrpgEngine() {
//     shutdown();
// }

// bool TrpgEngine::initializeEngine() {
//     std::cout << "Initializing TRPG Engine..." << std::endl;

//     // Initialize all subsystems (stubbed)
//     // m_graphicsEngine = new GraphicsEngine();
//     // m_audioEngine = new AudioEngine();
//     // m_physicsEngine = new PhysicsEngine();
//     // ...

//     m_sceneDirector = new SceneDirector();
//     m_resourceManager = new ResourceManager();

//     return true;
// }

// bool TrpgEngine::loadProjectData(const std::string& path) {
//     std::cout << "Loading project from: " << path << std::endl;
//     return m_resourceManager->loadProject(path);
// }

// bool TrpgEngine::saveProjectData(const std::string& path) {
//     std::cout << "Saving project to: " << path << std::endl;
//     return m_resourceManager->saveProject(path);
// }

// void TrpgEngine::startGame() {
//     std::cout << "Game started." << std::endl;
// }

// void TrpgEngine::pauseGame() {
//     std::cout << "Game paused." << std::endl;
// }

// void TrpgEngine::stopGame() {
//     std::cout << "Game stopped." << std::endl;
// }

// void TrpgEngine::updateFrame(float deltaTime) {
//     // Update subsystems
//     // m_gameLogicSystem->update(deltaTime);
//     // m_animationEngine->update(deltaTime);
//     // m_physicsEngine->update(deltaTime);
// }

// Scene* TrpgEngine::loadScene(const std::string& sceneName) {
//     return m_sceneDirector->loadScene(sceneName);
// }

// void TrpgEngine::setCurrentScene(Scene* scene) {
//     m_sceneDirector->setCurrentScene(scene);
// }

// GraphicsEngine* TrpgEngine::getGraphicsEngine() const {
//     return m_graphicsEngine;
// }

// AudioEngine* TrpgEngine::getAudioEngine() const {
//     return m_audioEngine;
// }

// PhysicsEngine* TrpgEngine::getPhysicsEngine() const {
//     return m_physicsEngine;
// }

// AnimationEngine* TrpgEngine::getAnimationEngine() const {
//     return m_animationEngine;
// }

// ResourceManager* TrpgEngine::getResourceManager() const {
//     return m_resourceManager;
// }

// UIManager* TrpgEngine::getUIManager() const {
//     return m_uiManager;
// }

// GameLogicSystem* TrpgEngine::getGameLogicSystem() const {
//     return m_gameLogicSystem;
// }

// DialogueSystem* TrpgEngine::getDialogueSystem() const {
//     return m_dialogueSystem;
// }

// EventSystem* TrpgEngine::getEventSystem() const {
//     return m_eventSystem;
// }

// ReplaySystem* TrpgEngine::getReplaySystem() const {
//     return m_replaySystem;
// }

// HUDSystem* TrpgEngine::getHUDSystem() const {
//     return m_hudSystem;
// }

// SceneDirector* TrpgEngine::getSceneDirector() const {
//     return m_sceneDirector;
// }

// void TrpgEngine::shutdown() {
//     std::cout << "Shutting down TRPG Engine..." << std::endl;

//     delete m_sceneDirector;
//     delete m_resourceManager;

//     // delete other subsystems if necessary
// }