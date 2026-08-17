#pragma once
#include <imgui_mk/windowRendereren.h>
#include <imgui_mk/ui/fui_core.h>
#include <utility>

// 函数式控件库·主题模块：Theme 纯值类型（颜色 + 几何），预设与链式派生。
// 应用粒度两层：
//   窗口级  applyTheme(ctx, theme) / window::setTheme(theme)
//           —— 每窗口独立 ImGui 上下文，天然按窗口异肤
//   子树级  styled(theme, view) 修饰器 —— PushStyleColor/Var 包裹作用域
//           styleVar(var, value, view) 单变量粒度
//
// 几何中的 itemSpacing 即全局控件间距：容器 vbox/hbox 的 spacing < 0（默认）
// 本就回落到 ImGui style，因此主题一改、所有容器间距跟随，无需逐处配置。
// 字体大小不进主题（字号烘焙进字体图集，运行时切换需重建图集，另行处理）。

namespace Mk::fui {

	// ============ Theme：纯值 ============

	struct Theme {
		// —— 颜色（ImVec4，w=alpha）——
		ImVec4 windowBg{0.11f, 0.11f, 0.13f, 1.00f};
		ImVec4 childBg{0.15f, 0.15f, 0.17f, 1.00f};
		ImVec4 popupBg{0.16f, 0.16f, 0.18f, 1.00f};
		ImVec4 frameBg{0.20f, 0.20f, 0.23f, 1.00f};
		ImVec4 frameBgHover{0.26f, 0.26f, 0.30f, 1.00f};
		ImVec4 frameBgActive{0.30f, 0.30f, 0.34f, 1.00f};
		ImVec4 text{0.92f, 0.92f, 0.94f, 1.00f};
		ImVec4 textDisabled{0.50f, 0.50f, 0.54f, 1.00f};
		ImVec4 button{0.24f, 0.26f, 0.31f, 1.00f};
		ImVec4 buttonHovered{0.30f, 0.33f, 0.39f, 1.00f};
		ImVec4 buttonActive{0.34f, 0.37f, 0.44f, 1.00f};
		ImVec4 header{0.19f, 0.21f, 0.25f, 1.00f};      // 列表选中/表头底
		ImVec4 headerHovered{0.25f, 0.27f, 0.32f, 1.00f};
		ImVec4 headerActive{0.28f, 0.31f, 0.36f, 1.00f};
		ImVec4 accent{0.26f, 0.59f, 0.98f, 1.00f};      // 主色：滑块柄/进度/选中强调
		ImVec4 accentHover{0.33f, 0.66f, 1.00f, 1.00f};
		ImVec4 accentActive{0.20f, 0.50f, 0.90f, 1.00f};
		ImVec4 border{0.28f, 0.28f, 0.32f, 0.60f};

		// —— 几何（像素）——
		float rounding = 6.0f;        // 窗口/弹窗圆角
		float frameRounding = 4.0f;   // 控件框架圆角
		float windowPadding = 8.0f;   // 窗口内边距
		float framePaddingX = 8.0f;   // 控件内边距 X
		float framePaddingY = 4.0f;   // 控件内边距 Y
		float itemSpacingX = 8.0f;    // 控件间距 X（容器默认间距来源）
		float itemSpacingY = 6.0f;    // 控件间距 Y
		float indentWidth = 20.0f;    // 缩进步进

		// —— 预设 ——
		static Theme dark() { return Theme{}; }

		static Theme light() {
			Theme t;
			t.windowBg    = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
			t.childBg     = ImVec4(0.90f, 0.90f, 0.91f, 1.00f);
			t.popupBg     = ImVec4(0.97f, 0.97f, 0.97f, 1.00f);
			t.frameBg     = ImVec4(0.88f, 0.88f, 0.90f, 1.00f);
			t.frameBgHover  = ImVec4(0.82f, 0.82f, 0.85f, 1.00f);
			t.frameBgActive = ImVec4(0.78f, 0.78f, 0.82f, 1.00f);
			t.text        = ImVec4(0.13f, 0.13f, 0.15f, 1.00f);
			t.textDisabled= ImVec4(0.55f, 0.55f, 0.58f, 1.00f);
			t.button      = ImVec4(0.84f, 0.85f, 0.88f, 1.00f);
			t.buttonHovered = ImVec4(0.78f, 0.80f, 0.84f, 1.00f);
			t.buttonActive  = ImVec4(0.73f, 0.76f, 0.81f, 1.00f);
			t.header      = ImVec4(0.82f, 0.85f, 0.91f, 1.00f);
			t.headerHovered = ImVec4(0.76f, 0.80f, 0.88f, 1.00f);
			t.headerActive  = ImVec4(0.72f, 0.77f, 0.86f, 1.00f);
			t.accent      = ImVec4(0.16f, 0.47f, 0.89f, 1.00f);
			t.accentHover = ImVec4(0.22f, 0.55f, 0.96f, 1.00f);
			t.accentActive= ImVec4(0.13f, 0.40f, 0.80f, 1.00f);
			t.border      = ImVec4(0.60f, 0.60f, 0.64f, 0.50f);
			return t;
		}

		// —— 链式派生（值语义，返回副本）——
		Theme withAccent(ImVec4 c) const {
			Theme t = *this;
			t.accent = c;
			t.accentHover = ImVec4(c.x * 1.12f, c.y * 1.12f, c.z * 1.12f, c.w);
			t.accentActive = ImVec4(c.x * 0.88f, c.y * 0.88f, c.z * 0.88f, c.w);
			return t;
		}
		Theme withRounding(float r) const { Theme t = *this; t.rounding = r; t.frameRounding = r * 0.66f; return t; }
		Theme withWindowPadding(float p) const { Theme t = *this; t.windowPadding = p; return t; }
		Theme withItemSpacing(float x, float y) const { Theme t = *this; t.itemSpacingX = x; t.itemSpacingY = y; return t; }
	};

	// ============ 应用：写入当前 ImGui 上下文 ============

	inline void applyTheme(ImGuiContext* ctx, const Theme& t) {
		ImGui::SetCurrentContext(ctx);
		ImGuiStyle& s = ImGui::GetStyle();

		s.WindowRounding = t.rounding;
		s.ChildRounding = t.rounding;
		s.PopupRounding = t.rounding;
		s.FrameRounding = t.frameRounding;
		s.GrabRounding = t.frameRounding;
		s.ScrollbarRounding = t.rounding;
		s.TabRounding = t.frameRounding;

		s.WindowPadding = ImVec2(t.windowPadding, t.windowPadding);
		s.FramePadding = ImVec2(t.framePaddingX, t.framePaddingY);
		s.ItemSpacing = ImVec2(t.itemSpacingX, t.itemSpacingY);
		s.IndentSpacing = t.indentWidth;
		s.GrabMinSize = 10.0f;
		s.ScrollbarSize = 14.0f;
		s.WindowBorderSize = 1.0f;
		s.ChildBorderSize = 1.0f;
		s.FrameBorderSize = 0.0f;
		s.WindowTitleAlign = ImVec2(0.0f, 0.5f);
		s.ButtonTextAlign = ImVec2(0.5f, 0.5f);

		ImVec4* c = s.Colors;
		c[ImGuiCol_Text] = t.text;
		c[ImGuiCol_TextDisabled] = t.textDisabled;
		c[ImGuiCol_WindowBg] = t.windowBg;
		c[ImGuiCol_ChildBg] = t.childBg;
		c[ImGuiCol_PopupBg] = ImVec4(t.popupBg.x, t.popupBg.y, t.popupBg.z, 0.98f);
		c[ImGuiCol_Border] = t.border;
		c[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
		c[ImGuiCol_FrameBg] = t.frameBg;
		c[ImGuiCol_FrameBgHovered] = t.frameBgHover;
		c[ImGuiCol_FrameBgActive] = t.frameBgActive;
		c[ImGuiCol_TitleBg] = t.header;
		c[ImGuiCol_TitleBgActive] = t.headerActive;
		c[ImGuiCol_TitleBgCollapsed] = t.header;
		c[ImGuiCol_MenuBarBg] = t.header;
		c[ImGuiCol_ScrollbarBg] = t.frameBg;
		c[ImGuiCol_ScrollbarGrab] = t.border;
		c[ImGuiCol_ScrollbarGrabHovered] = t.textDisabled;
		c[ImGuiCol_ScrollbarGrabActive] = t.text;
		c[ImGuiCol_CheckMark] = t.accent;
		c[ImGuiCol_SliderGrab] = t.accent;
		c[ImGuiCol_SliderGrabActive] = t.accentActive;
		c[ImGuiCol_Button] = t.button;
		c[ImGuiCol_ButtonHovered] = t.buttonHovered;
		c[ImGuiCol_ButtonActive] = t.buttonActive;
		c[ImGuiCol_Header] = t.header;
		c[ImGuiCol_HeaderHovered] = t.headerHovered;
		c[ImGuiCol_HeaderActive] = t.headerActive;
		c[ImGuiCol_Separator] = t.border;
		c[ImGuiCol_SeparatorHovered] = t.headerHovered;
		c[ImGuiCol_SeparatorActive] = t.accent;
		c[ImGuiCol_ResizeGrip] = t.border;
		c[ImGuiCol_ResizeGripHovered] = t.accentHover;
		c[ImGuiCol_ResizeGripActive] = t.accentActive;
		c[ImGuiCol_Tab] = t.header;
		c[ImGuiCol_TabHovered] = t.accentHover;
		c[ImGuiCol_TabActive] = t.accent;
		c[ImGuiCol_TabUnfocused] = t.header;
		c[ImGuiCol_TabUnfocusedActive] = t.headerHovered;
		c[ImGuiCol_PlotLines] = t.accent;
		c[ImGuiCol_PlotHistogram] = t.accent;
		c[ImGuiCol_TableHeaderBg] = t.header;
		c[ImGuiCol_TableBorderStrong] = t.border;
		c[ImGuiCol_TableBorderLight] = ImVec4(t.border.x, t.border.y, t.border.z, t.border.w * 0.5f);
		c[ImGuiCol_TextSelectedBg] = ImVec4(t.accent.x, t.accent.y, t.accent.z, 0.35f);
		c[ImGuiCol_DragDropTarget] = t.accent;
		c[ImGuiCol_NavCursor] = t.accent;
		c[ImGuiCol_ModalWindowDimBg] = ImVec4(0.10f, 0.10f, 0.12f, 0.55f);
	}

	// ============ 子树作用域修饰器 ============

	// 主题作用域换肤：Push 全部主题色 + 几何，子树渲染后配对 Pop。
	// 典型用途：局部 accent 覆盖、紧凑/宽松区域（withItemSpacing 派生）
	template<typename T>
	struct Styled {
		Theme theme;
		T child;
	};

	template<typename T>
	void draw(const Styled<T>& w);

	namespace detail {
		// push 主题（26 色 + 3 变量）；draw(Styled) 内 Pop 数量与此配对，改动时两边同步
		inline int pushTheme(const Theme& t) {
			ImGuiStyle& s = ImGui::GetStyle();
			ImGui::PushStyleColor(ImGuiCol_Text, t.text);
			ImGui::PushStyleColor(ImGuiCol_TextDisabled, t.textDisabled);
			ImGui::PushStyleColor(ImGuiCol_WindowBg, t.windowBg);
			ImGui::PushStyleColor(ImGuiCol_ChildBg, t.childBg);
			ImGui::PushStyleColor(ImGuiCol_PopupBg, t.popupBg);
			ImGui::PushStyleColor(ImGuiCol_Border, t.border);
			ImGui::PushStyleColor(ImGuiCol_FrameBg, t.frameBg);
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, t.frameBgHover);
			ImGui::PushStyleColor(ImGuiCol_FrameBgActive, t.frameBgActive);
			ImGui::PushStyleColor(ImGuiCol_TextSelectedBg,
				ImVec4(t.accent.x, t.accent.y, t.accent.z, 0.35f));
			ImGui::PushStyleColor(ImGuiCol_Button, t.button);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, t.buttonHovered);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, t.buttonActive);
			ImGui::PushStyleColor(ImGuiCol_Header, t.header);
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, t.headerHovered);
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, t.headerActive);
			ImGui::PushStyleColor(ImGuiCol_CheckMark, t.accent);
			ImGui::PushStyleColor(ImGuiCol_SliderGrab, t.accent);
			ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, t.accentActive);
			ImGui::PushStyleColor(ImGuiCol_Separator, t.border);
			ImGui::PushStyleColor(ImGuiCol_PlotLines, t.accent);
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, t.accent);
			ImGui::PushStyleColor(ImGuiCol_Tab, t.header);
			ImGui::PushStyleColor(ImGuiCol_TabHovered, t.accentHover);
			ImGui::PushStyleColor(ImGuiCol_TabActive, t.accent);
			ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, t.header);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, t.frameRounding);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(t.framePaddingX, t.framePaddingY));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(t.itemSpacingX, t.itemSpacingY));
			(void)s;
			return 0;
		}
	}

	template<typename T>
	void draw(const Styled<T>& w) {
		detail::pushTheme(w.theme);
		draw(w.child);
		ImGui::PopStyleColor(26);
		ImGui::PopStyleVar(3);
	}

	// 单样式变量作用域（float 或 ImVec2 重载由 ImGui 的 PushStyleVar 双签名分流）
	template<typename T>
	struct StyleVarF {
		ImGuiStyleVar idx;
		float value;
		T child;
	};
	template<typename T>
	struct StyleVarV {
		ImGuiStyleVar idx;
		ImVec2 value;
		T child;
	};

	template<typename T> void draw(const StyleVarF<T>& w);
	template<typename T> void draw(const StyleVarV<T>& w);

	template<typename T>
	void draw(const StyleVarF<T>& w) {
		ImGui::PushStyleVar(w.idx, w.value);
		draw(w.child);
		ImGui::PopStyleVar();
	}
	template<typename T>
	void draw(const StyleVarV<T>& w) {
		ImGui::PushStyleVar(w.idx, w.value);
		draw(w.child);
		ImGui::PopStyleVar();
	}

	// ============ 工厂 ============

	template<typename T>
	auto styled(Theme theme, T child) {
		return Styled<std::decay_t<T>>{std::move(theme), std::move(child)};
	}
	template<typename T>
	auto styleVar(ImGuiStyleVar idx, float value, T child) {
		return StyleVarF<std::decay_t<T>>{idx, value, std::move(child)};
	}
	template<typename T>
	auto styleVar(ImGuiStyleVar idx, const ImVec2& value, T child) {
		return StyleVarV<std::decay_t<T>>{idx, value, std::move(child)};
	}

}