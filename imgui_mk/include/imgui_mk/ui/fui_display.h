#pragma once
#include <imgui_mk/ui/fui_core.h>
#include <cfloat>
#include <cstdint>
#include <vector>

// 函数式控件库·显示反馈模块：文本展示、进度条、分隔、可选中行、图片、折线图、
// 树节点容器，以及 tooltip/colored/indent 修饰器与 OpenGL 纹理上传辅助。

namespace Mk::fui {

	// —— 显示控件 ——

	struct TextColored { std::string text; ImVec4 color; };
	inline void draw(const TextColored& w) { ImGui::TextColored(w.color, "%s", w.text.c_str()); }

	struct TextWrapped { std::string text; };
	inline void draw(const TextWrapped& w) { ImGui::TextWrapped("%s", w.text.c_str()); }

	struct BulletText { std::string text; };
	inline void draw(const BulletText& w) { ImGui::BulletText("%s", w.text.c_str()); }

	// 标签+值成对显示（右对齐值风格）
	struct LabelText { std::string label; std::string text; };
	inline void draw(const LabelText& w) { ImGui::LabelText(w.label.c_str(), "%s", w.text.c_str()); }

	// 进度条：绑定外部 fraction（0~1）
	struct ProgressBar { float& fraction; ImVec2 size; std::string overlay; };
	inline void draw(const ProgressBar& w) {
		ImGui::ProgressBar(w.fraction, w.size,
			w.overlay.empty() ? nullptr : w.overlay.c_str());
	}

	struct Separator {};
	inline void draw(const Separator&) { ImGui::Separator(); }

	struct SeparatorText { std::string label; };
	inline void draw(const SeparatorText& w) { ImGui::SeparatorText(w.label.c_str()); }

	// 可选中行：绑定外部 bool（选中态）
	struct Selectable { std::string label; bool& selected; };
	inline void draw(const Selectable& w) { ImGui::Selectable(w.label.c_str(), &w.selected); }

	// 图片：tex 经 loadTextureRGBA 获得（OpenGL 纹理句柄的 ImTextureID 封装）
	struct Image { ImTextureID tex; ImVec2 size; ImVec4 tint; ImVec4 border; };
	inline void draw(const Image& w) {
		ImGui::Image(w.tex, w.size, ImVec2(0, 0), ImVec2(1, 1), w.tint, w.border);
	}

	struct ImageButton { std::string id; ImTextureID tex; ImVec2 size; std::function<void()> onClick; };
	inline void draw(const ImageButton& w) {
		if(ImGui::ImageButton(w.id.c_str(), w.tex, w.size) && w.onClick) w.onClick();
	}

	// 折线图：绑定外部历史数据（const 引用，逻辑侧维护）
	struct PlotLines { std::string label; const std::vector<float>& values; float mn, mx; ImVec2 size; };
	inline void draw(const PlotLines& w) {
		ImGui::PlotLines(w.label.c_str(), w.values.data(), (int)w.values.size(),
			0, nullptr, w.mn, w.mx, w.size);
	}

	// —— 树节点容器与修饰器 ——

	// 树节点容器：TreeNodeEx 展开时渲染子视图，收起时跳过
	template<typename... Ts>
	struct TreeNode {
		std::string label;
		ImGuiTreeNodeFlags flags;
		std::tuple<Ts...> children;
	};

	// 悬停提示：子视图渲染后，鼠标悬停在该 item 上时显示提示文本
	template<typename T>
	struct Tooltip { T child; std::string text; };

	// 颜色修饰：PushStyleColor 包裹子视图（给一段任意视图上色）
	template<typename T>
	struct Colored { ImGuiCol idx; ImVec4 color; T child; };

	// 缩进修饰：子视图整体缩进 amount 像素
	template<typename T>
	struct Indent { float amount; T child; };

	// draw 重载：先声明、再定义（见 fui_core.h 的说明）
	template<typename... Ts> void draw(const TreeNode<Ts...>& w);
	template<typename T> void draw(const Tooltip<T>& w);
	template<typename T> void draw(const Colored<T>& w);
	template<typename T> void draw(const Indent<T>& w);

	template<typename... Ts>
	void draw(const TreeNode<Ts...>& w) {
		if(ImGui::TreeNodeEx(w.label.c_str(), w.flags)) {
			std::apply([&](const auto&... cs) {
				const auto one = [](const auto& c) { draw(c); };
				(one(cs), ...);
			}, w.children);
			ImGui::TreePop();
		}
	}

	template<typename T>
	void draw(const Tooltip<T>& w) {
		draw(w.child);
		if(ImGui::IsItemHovered()) ImGui::SetTooltip("%s", w.text.c_str());
	}

	template<typename T>
	void draw(const Colored<T>& w) {
		ImGui::PushStyleColor(w.idx, w.color);
		draw(w.child);
		ImGui::PopStyleColor();
	}

	template<typename T>
	void draw(const Indent<T>& w) {
		ImGui::Indent(w.amount);
		draw(w.child);
		ImGui::Unindent(w.amount);
	}

	// —— 工厂 ——

	inline auto textColored(std::string text, const ImVec4& color) {
		return TextColored{std::move(text), color};
	}
	inline auto textWrapped(std::string text) { return TextWrapped{std::move(text)}; }
	inline auto bulletText(std::string text) { return BulletText{std::move(text)}; }
	inline auto labelText(std::string label, std::string text) {
		return LabelText{std::move(label), std::move(text)};
	}
	inline auto progressBar(float& fraction, ImVec2 size = ImVec2(-FLT_MIN, 0),
		std::string overlay = "") {
		return ProgressBar{fraction, size, std::move(overlay)};
	}
	inline auto separator() { return Separator{}; }
	inline auto separatorText(std::string label) { return SeparatorText{std::move(label)}; }
	inline auto selectable(std::string label, bool& selected) {
		return Selectable{std::move(label), selected};
	}
	inline auto image(ImTextureID tex, ImVec2 size,
		const ImVec4& tint = ImVec4(1, 1, 1, 1), const ImVec4& border = ImVec4(0, 0, 0, 0)) {
		return Image{tex, size, tint, border};
	}
	inline auto imageButton(std::string id, ImTextureID tex, ImVec2 size,
		std::function<void()> onClick = nullptr) {
		return ImageButton{std::move(id), tex, size, std::move(onClick)};
	}
	inline auto plotLines(std::string label, const std::vector<float>& values,
		ImVec2 size = ImVec2(0, 60), float mn = FLT_MAX, float mx = FLT_MAX) {
		return PlotLines{std::move(label), values, mn, mx, size};
	}
	template<typename... Ts>
	auto treeNode(std::string label, ImGuiTreeNodeFlags flags, Ts&&... views) {
		return TreeNode<std::decay_t<Ts>...>{
			std::move(label), flags, std::make_tuple(std::forward<Ts>(views)...)};
	}
	template<typename T>
	auto tooltip(T child, std::string text) {
		return Tooltip<std::decay_t<T>>{std::move(child), std::move(text)};
	}
	template<typename T>
	auto colored(ImGuiCol idx, const ImVec4& color, T child) {
		return Colored<std::decay_t<T>>{idx, color, std::move(child)};
	}
	template<typename T>
	auto indent(float amount, T child) { return Indent<std::decay_t<T>>{amount, std::move(child)}; }

	// —— 纹理辅助（OpenGL）——
	// RGBA8 像素上传为纹理，返回可传给 image/imageButton 的 ImTextureID。
	// 须在窗口/GL 上下文存活期间调用；释放用 deleteTexture
	inline ImTextureID loadTextureRGBA(const unsigned char* pixels, int width, int height) {
		GLuint tex = 0;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		return (ImTextureID)(intptr_t)tex;
	}

	inline void deleteTexture(ImTextureID tex) {
		GLuint t = (GLuint)(intptr_t)tex;
		glDeleteTextures(1, &t);
	}

}