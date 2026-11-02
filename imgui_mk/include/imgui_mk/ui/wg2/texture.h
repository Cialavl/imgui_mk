#pragma once
#include <glad/glad.h>
#include <imgui.h>

#include <cstdint>
#include <unordered_map>

// Mk::wg2 纹理：RGBA8 上传 / 删除 OpenGL 纹理 + 引擎默认纹理。
//   默认纹理由引擎持有（每个窗口的 GL 上下文一份，懒创建）：
//   控件没设 tex 时用它，应用不必再自己准备占位图。

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

namespace detail {

// 程序生成的 16x16 棋盘：图像控件没设纹理时的占位内容（不依赖外部文件）
inline ImTextureID createPlaceholderTexture() {
	static unsigned char px[16 * 16 * 4];
	for (int y = 0; y < 16; ++y)
		for (int x = 0; x < 16; ++x) {
			const bool odd = ((x / 4) + (y / 4)) % 2 == 1;
			unsigned char* p = &px[(y * 16 + x) * 4];
			p[0] = p[1] = p[2] = static_cast<unsigned char>(odd ? 190 : 110);
			p[3] = 255;
		}
	return loadTextureRGBA(px, 16, 16);
}

inline std::unordered_map<ImGuiContext*, ImTextureID>& defaultTextureStore() {
	static std::unordered_map<ImGuiContext*, ImTextureID> store;
	return store;
}

} // namespace detail

// 引擎默认纹理：首次使用时生成并登记（每个上下文一份）。
// 没有渲染后端（离线单元测试）时返回 0，不触碰 GL。
inline ImTextureID defaultTexture() {
	ImGuiContext* ctx = ImGui::GetCurrentContext();
	if (!ctx) return 0;
	auto& store = detail::defaultTextureStore();
	const auto it = store.find(ctx);
	if (it != store.end()) return it->second;
	if (!ImGui::GetIO().BackendRendererUserData) return 0;
	const ImTextureID tex = detail::createPlaceholderTexture();
	store[ctx] = tex;
	return tex;
}

// 应用可覆盖引擎默认纹理（换成自己的占位图）
inline void setDefaultTexture(ImTextureID tex) {
	if (ImGuiContext* ctx = ImGui::GetCurrentContext()) detail::defaultTextureStore()[ctx] = tex;
}

// 控件取纹理的统一入口：自己设了用自己，没设用引擎默认值
inline ImTextureID textureOr(ImTextureID tex) { return tex ? tex : defaultTexture(); }

// 清空登记（窗口重建 / 测试夹具）
inline void clearTextures() { detail::defaultTextureStore().clear(); }

} // namespace Mk::wg2
