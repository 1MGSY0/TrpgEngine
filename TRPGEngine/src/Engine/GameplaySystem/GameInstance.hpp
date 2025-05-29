#pragma once
#include "Engine/EntitySystem/Entity.hpp"

class GameInstance {
public:
    static GameInstance& get();

    void startGame();             // Called when "Run" is pressed
    void update(float deltaTime); // Call per frame
    void reset();                 // Stop game and return to editor

    bool isRunning() const { return m_running; }

private:
    bool m_running = false;
};
