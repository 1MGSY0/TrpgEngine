#pragma once

#include <string>
#include "Engine/EntitySystem/Entity.hpp"

namespace RenderSystem {
	// Initialize / shutdown (no-op for now, present for later expansion)
	void init();
	void shutdown();

	// Editor preview / runtime render entry points (minimal stubs)
	void renderEntityEditor(Entity e);
	void renderEntityRuntime(Entity e);

	// Scene management
	void beginScene();
	void endScene();
	void renderScenePanel();

	// Texture management
	void invalidateTexture(const std::string& key);
}
