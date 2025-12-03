#pragma once
#include <imgui.h>
#include <string>

namespace ResourceUtils {
	// Ensure any GL/texture placeholders are created. Safe to call multiple times.
	void ensureInitialized();

	// Return an ImTextureID suitable for ImGui::Image usage (placeholder until real loader exists).
	ImTextureID getPlaceholderTexture();

	// Try to load a texture from disk; currently a stub that returns the placeholder.
	// Later this will attempt filesystem load and upload GL texture.
	ImTextureID loadTextureFromFile(const std::string& path);
}
