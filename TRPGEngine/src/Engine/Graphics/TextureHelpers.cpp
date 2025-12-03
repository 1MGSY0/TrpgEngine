#include "Engine/Graphics/TextureHelpers.hpp"
#include <glad/glad.h>
#include <iostream>

namespace Graphics::TextureHelpers {
	unsigned int createTextureFromRGBA(int width, int height, const unsigned char* pixels) {
		if (width <= 0 || height <= 0 || pixels == nullptr) return 0u;

		unsigned int tex = 0u;
		glGenTextures(1, &tex);
		if (tex == 0u) return 0u;

		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		// Check for OpenGL errors
		GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
			std::cerr << "[TextureHelpers] OpenGL error while creating texture: " << error << "\n";
			glDeleteTextures(1, &tex);
			return 0;
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		return tex;
	}
}
