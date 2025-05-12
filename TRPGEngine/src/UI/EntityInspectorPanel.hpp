#pragma once

#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Entity.hpp"

// Forward declarations for known component inspectors
class CharacterComponent;
class ScriptComponent;

void renderCharacterInspector(std::shared_ptr<CharacterComponent> character);
void renderScriptInspector(std::shared_ptr<ScriptComponent> script);

/**
 * Renders the Inspector UI for a given entity.
 * Dynamically displays known components (Character, Script, etc.)
 * and ignores unknown ones unless extended.
 *
 * @param entity The selected entity.
 * @param em Reference to the active EntityManager.
 */

void renderEntityInspector(Entity entity, EntityManager& em);
