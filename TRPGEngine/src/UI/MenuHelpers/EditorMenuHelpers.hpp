#pragma once
#include <memory>
#include <json.hpp>
#include "Engine/EntitySystem/Entity.hpp"
#include "Engine/EntitySystem/ComponentType.hpp"

namespace EditorMenuHelpers {
// Returns selected FlowNode; if an event is selected, returns its owning FlowNode; otherwise INVALID_ENTITY.
Entity getPreferredSceneNode();

// Creates a new FlowNode (scene), registers it in ProjectMeta, links from selected scene if empty next, and selects it.
Entity createSceneFlowNode();

// Finds the owning scene of an event by scanning ProjectMeta sceneNodes' eventSequence.
Entity findOwnerScene(Entity evt);

// Attaches evt to sceneNode->eventSequence (removes from previous owner if present). Marks unsaved and refreshes SceneManager.
bool attachEventToScene(Entity evt, Entity sceneNode);

// Creates a new event entity by ComponentType, initializes default JSON, attaches to scene, selects it, and marks unsaved.
Entity createEventAndAttach(ComponentType type, Entity sceneNode);
}
