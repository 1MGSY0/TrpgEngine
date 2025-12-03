#include "ResourceUtils.hpp"
#include "Engine/RenderSystem/RenderSystem.hpp"
#include "Engine/Graphics/TextureHelpers.hpp"
#include "Resources/ResourceManager.hpp"
#include <glad/glad.h>
#include <iostream>
#include <mutex>
#include <filesystem>
#include <cstdint>
// stb_image implementation
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace ResourceUtils {

namespace {
ImTextureID loadTextureFromAbsolutePath(const std::string& absPath);
} // namespace

static GLuint s_placeholderTex = 0;
static std::once_flag s_initFlag;

void ensureInitialized() {
    std::call_once(s_initFlag, [](){
        std::filesystem::path baseDir = std::filesystem::current_path();
        std::filesystem::path placeholderPath = baseDir / "Runtime" / "Assets" / "Backgrounds" / "no-image-icon-6.png";

        std::cout << "[ResourceUtils] Attempting to load no-image-icon-6.png from: " << placeholderPath << "\n";

        if (!placeholderPath.empty() && std::filesystem::exists(placeholderPath)) {
            if (ImTextureID tex = loadTextureFromAbsolutePath(placeholderPath.generic_string())) {
                uintptr_t texPtr = static_cast<uintptr_t>(tex);
                if (texPtr != 0u) {
                    s_placeholderTex = static_cast<GLuint>(texPtr);
                    std::cout << "[ResourceUtils] Successfully loaded no-image-icon-6.png as placeholder texture.\n";
                    return;
                }
            }
            std::cout << "[ResourceUtils] Failed to load no-image-icon-6.png as texture.\n";
        } else {
            std::cout << "[ResourceUtils] File not found: " << placeholderPath << "\n";
        }

        // Fallback to blue placeholder
        glGenTextures(1, &s_placeholderTex);
        glBindTexture(GL_TEXTURE_2D, s_placeholderTex);
        unsigned char blue[4] = { 0x00, 0x00, 0xFF, 0xFF }; // RGBA: Blue
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, blue);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);
        std::cout << "[ResourceUtils] Created blue placeholder texture (id=" << s_placeholderTex << ")\n";
    });
}

ImTextureID getPlaceholderTexture() {
	ensureInitialized();
	return (ImTextureID)(intptr_t)s_placeholderTex;
}

namespace {
ImTextureID loadTextureFromAbsolutePath(const std::string& absPath) {
	if (absPath.empty()) return (ImTextureID)0;

	std::filesystem::path filePath(absPath);
	std::string extension = filePath.extension().string();
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

	if (extension != ".jpg" && extension != ".jpeg" && extension != ".png" && extension != ".bmp") {
		std::cerr << "[ResourceUtils] Unsupported file extension: " << extension << " for file: " << absPath << "\n";
		return (ImTextureID)0;
	}

	stbi_set_flip_vertically_on_load(false);
	int width = 0, height = 0, channels = 0;

	// Attempt to load the image
	stbi_uc* pixels = stbi_load(absPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if (!pixels || width <= 0 || height <= 0) {
		std::cerr << "[ResourceUtils] stbi_load failed for: " << absPath << "\n";
		std::cerr << "[ResourceUtils] stbi_failure_reason: " << stbi_failure_reason() << "\n";
		if (pixels) stbi_image_free(pixels);
		return (ImTextureID)0;
	}

	std::cout << "[ResourceUtils] Loaded image: " << absPath << " (Width: " << width << ", Height: " << height << ", Channels: " << channels << ")\n";

	// Create the texture
	ImTextureID tex = (ImTextureID)(intptr_t)Graphics::TextureHelpers::createTextureFromRGBA(width, height, pixels);
	stbi_image_free(pixels);

	if (!tex) {
		std::cerr << "[ResourceUtils] Failed to create texture from: " << absPath << "\n";
	}
	return tex ? tex : (ImTextureID)0;
}
} // namespace

ImTextureID loadTextureFromFile(const std::string& fileName) {
    ensureInitialized();

    if (fileName.empty()) {
        std::cout << "[ResourceUtils] Empty fileName, using placeholder texture.\n";
        return getPlaceholderTexture();
    }
	std::filesystem::path baseDir = std::filesystem::current_path();
    std::filesystem::path fullPath = baseDir / "Runtime" / "Assets" / "Backgrounds" / fileName;
    std::cout << "[ResourceUtils] Loading background from: " << fullPath << "\n";

    if (ImTextureID tex = loadTextureFromAbsolutePath(fullPath.generic_string())) {
        return tex;
    }

    std::cout << "[ResourceUtils] Failed to load '" << fileName
              << "', using placeholder.\n";
    return getPlaceholderTexture();
}
} // namespace ResourceUtils
