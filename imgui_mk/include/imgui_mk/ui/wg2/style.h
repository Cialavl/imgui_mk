#pragma once
#include <imgui_mk/windowRendereren.h>
#include <utility>

// Mk::wg2 主题模块：Theme 纯值类型（颜色 + 几何），预设与链式派生。
// 应用粒度两层：
//   窗口级  applyTheme(ctx, theme) / window::setTheme(theme)
//           —— 每窗口独立 ImGui 上下文，天然按窗口异肤
//   子树级  Styled 控件（wg_controls.h）—— PushStyleColor/Var 包裹作用域
//
// 几何中的 itemSpacing 即全局控件间距：VBox/HBox 的 spacing < 0（默认）
// 本就回落到 ImGui style，因此主题一改、所有容器间距跟随，无需逐处配置。
// 几何随 fontSize 等比联动（见 Theme 字段与 applyTheme）；字体位图本身在窗口创建时按 fontSize 烘焙，setTheme 运行时切换不重建图集。

namespace Mk::wg2 {

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

		// —— 几何 ——
		// 圆角为绝对像素；间距/内边距字段默认 -1 = 按字号比值推导（见 applyTheme），
		// 显式写入绝对值（withItemSpacing 等）则优先于比例——显式意图覆盖自动推导
		float rounding = 6.0f;        // 窗口/弹窗圆角（绝对像素）
		float frameRounding = 4.0f;   // 控件框架圆角（绝对像素）
		float windowPadding = -1.0f;  // 窗口内边距；<0 → 0.40×fontSize
		float framePaddingX = -1.0f;  // 控件内边距 X；<0 → 0.38×fontSize
		float framePaddingY = -1.0f;  // 控件内边距 Y；<0 → 0.16×fontSize
		float itemSpacingX = -1.0f;   // 控件间距 X；<0 → 0.42×fontSize（容器默认间距来源）
		float itemSpacingY = -1.0f;   // 控件间距 Y；<0 → 0.23×fontSize
		float indentWidth = -1.0f;    // 缩进步进；<0 → 0.90×fontSize
		float fontSize = 18.0f;       // 基准字号：改此值整套 UI（文字/控件/间距/边距）等比联动

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
		Theme withFontSize(float s) const { Theme t = *this; t.fontSize = s; return t; }
	};

	// ============ 应用：写入当前 ImGui 上下文 ============

	inline void applyTheme(ImGuiContext* ctx, const Theme& t) {
		ImGuiContext* prev = ImGui::GetCurrentContext();   // 保存调用方上下文：跨窗口调用（其他窗口帧内）不污染
		ImGui::SetCurrentContext(ctx);
		ImGuiStyle& s = ImGui::GetStyle();

		s.WindowRounding = t.rounding;
		s.ChildRounding = t.rounding;
		s.PopupRounding = t.rounding;
		s.FrameRounding = t.frameRounding;
		s.GrabRounding = t.frameRounding;
		s.ScrollbarRounding = t.rounding;
		s.TabRounding = t.frameRounding;

		// 几何推导：字段 >=0 用绝对值（显式覆盖），否则按 基准字号×比值。
		// 比值按紧凑桌面密度标定（@18px ≈ 边距 8.3 / 行距 8.6×4.7 / 控件内距 7.2×3.1）：
		// 相比早先版本整体收紧约 20%，避免嵌套容器层层叠加内边距显得空旷。
		const float F = t.fontSize;
		const auto pick = [](float absOrSentinel, float ratio, float fontSize_) {
			return absOrSentinel >= 0.0f ? absOrSentinel : ratio * fontSize_;
		};
		s.WindowPadding = ImVec2(pick(t.windowPadding, 0.40f, F), pick(t.windowPadding, 0.40f, F));
		s.FramePadding = ImVec2(pick(t.framePaddingX, 0.38f, F), pick(t.framePaddingY, 0.16f, F));
		s.ItemSpacing = ImVec2(pick(t.itemSpacingX, 0.42f, F), pick(t.itemSpacingY, 0.23f, F));
		s.IndentSpacing = pick(t.indentWidth, 0.90f, F);
		s.GrabMinSize = 0.55f * F;
		s.ScrollbarSize = 0.78f * F;
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
		ImGui::SetCurrentContext(prev);
	}

	// ============ 默认主题与 DPI ============

	namespace detail {
		// 全局默认主题槽（函数局部 static，跨 TU 合一；窗口构造时读取）
		inline Theme& defaultThemeSlot() {
			static Theme t = Theme::dark();
			return t;
		}

		// push 主题（26 色 + 3 变量）；Styled 控件（wg_controls.h）内 Pop 数量与此配对
		inline void pushTheme(const Theme& t) {
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
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
				ImVec2(t.framePaddingX >= 0.0f ? t.framePaddingX : 0.38f * t.fontSize,
					t.framePaddingY >= 0.0f ? t.framePaddingY : 0.16f * t.fontSize));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
				ImVec2(t.itemSpacingX >= 0.0f ? t.itemSpacingX : 0.42f * t.fontSize,
					t.itemSpacingY >= 0.0f ? t.itemSpacingY : 0.23f * t.fontSize));
		}
	} // namespace detail

	// 当前全局默认主题（未设置过则 dark@18）
	inline const Theme& defaultTheme() { return detail::defaultThemeSlot(); }

	// 设置全局默认主题：须在首个窗口创建前调用——窗口构造时读取它烘焙
	// 字体位图（fontSize）与应用样式；之后切换只影响新窗口
	inline void setDefaultTheme(const Theme& t) { detail::defaultThemeSlot() = t; }

	// 主显示器 DPI 缩放（进程内缓存；glfwInit 幂等可先调用；取不到则 1.0）。
	// 默认不参与任何路径——测试保持确定性，应用显式 opt-in
	inline float dpiScale() {
		static const float cached = [] {
			if(!glfwInit()) return 1.0f;
			if(GLFWmonitor* m = glfwGetPrimaryMonitor()) {
				float x = 1.0f, y = 1.0f;
				glfwGetMonitorContentScale(m, &x, &y);
				return (x > 0.0f && y > 0.0f) ? (x + y) * 0.5f : 1.0f;
			}
			return 1.0f;
		}();
		return cached;
	}

	// 按显示器 DPI 派生主题：字号×scale，几何随字号自动等比；
	// 窗口默认尺寸建议同步 ×dpiScale() 保持物理观感一致
	inline Theme scaledForMonitor(const Theme& t) {
		Theme r = t;
		r.fontSize = t.fontSize * dpiScale();
		return r;
	}

} // namespace Mk::wg2
