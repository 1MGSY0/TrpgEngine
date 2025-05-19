#pragma once

#include <memory>
#include <vector>
#include <string>
#include <imgui.h> 
#include <Engine/EntitySystem/Entity.hpp>
#include <Engine/EntitySystem/Components/DialogueComponent.hpp>


class ScenePanel {
public:
    ScenePanel();
    ~ScenePanel();

    void renderScenePanel();                             // Call every frame
    void renderTextOverlay(const DialogueComponent& text); // For dialogues and narration

private:
    ImVec2 m_panelSize;                        // For layout reference
    ImVec2 m_origin;                           // Top-left of scene window

    void renderEntity(Entity entity);
    void handleDragDrop();
};
