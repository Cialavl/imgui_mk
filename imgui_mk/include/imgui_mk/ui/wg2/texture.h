#pragma once
#include <glad/glad.h>
#include <imgui.h>

// Mk::wg2 纹理工具：RGBA8 上传 / 删除 OpenGL 纹理。

namespace Mk::wg2 {

inline ImTextureID loadTextureRGBA(const unsigned char* pixels, int width, int height) {
	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	return (ImTextureID)(intptr_t)tex;
}

inline void deleteTexture(ImTextureID tex) {
	GLuint t = (GLuint)(intptr_t)tex;
	glDeleteTextures(1, &t);
}

} // namespace Mk::wg2
