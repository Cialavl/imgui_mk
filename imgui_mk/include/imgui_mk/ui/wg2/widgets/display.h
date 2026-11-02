#pragma once
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <string>
#include <vector>
#include <imgui.h>
#include <imgui_mk/ui/wg2/core.h>
#include <imgui_mk/ui/wg2/texture.h>   // 引擎默认纹理：控件没设 tex 时的兜底
#include <imgui_mk/ui/wg2/widget.h>

// Mk::wg2 显示类原子控件：直接 template struct。

namespace Mk::wg2 {

template<FixedString Id>
struct ProgressBar {
	static constexpr auto id = Id;

	float* fraction = nullptr;
	// 拉取式比例：设置后每帧调用（动画/派生值无需外部变量）
	std::function<float()> fractionFn;
	std::string overlay;

	Size hint(const Size& avail) const {
		return Size{avail.w > 0.0f ? avail.w : 200.0f, ImGui::GetFrameHeight()};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		const float f = fractionFn ? fractionFn() : (fraction ? *fraction : 0.0f);
		if (!fractionFn && !fraction) return;
		ImGui::ProgressBar(f, ImVec2(r.w, r.h),
			overlay.empty() ? nullptr : overlay.c_str());
	}
};

template<FixedString Id>
struct TextWrapped {
	static constexpr auto id = Id;

	std::string text;

	Size hint(const Size& avail) const {
		const ImVec2 s = ImGui::CalcTextSize(text.c_str(), nullptr, true, avail.w);
		return Size{avail.w, s.y};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + r.w);
		ImGui::TextUnformatted(text.c_str());
		ImGui::PopTextWrapPos();
	}
};

template<FixedString Id>
struct BulletText {
	static constexpr auto id = Id;

	std::string text;

	Size hint(const Size&) const {
		const ImVec2 s = ImGui::CalcTextSize(text.c_str());
		return Size{s.x + 16.0f, s.y};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect&) {
		ImGui::BulletText("%s", text.c_str());
	}
};

template<FixedString Id>
struct LabelText {
	static constexpr auto id = Id;

	std::string name;
	std::string value;
	std::function<std::string()> valueFn;

	template<class F>
	void setValueFn(F&& fn) { valueFn = std::forward<F>(fn); }

	Size hint(const Size&) const {
		const std::string shown = valueFn ? valueFn() : value;
		const ImVec2 v = ImGui::CalcTextSize(shown.c_str());
		const float nameW = ImGui::CalcTextSize(name.c_str()).x;
		return Size{v.x + nameW + 8.0f, v.y};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect&) {
		const std::string shown = valueFn ? valueFn() : value;
		ImGui::TextUnformatted(shown.c_str());
		ImGui::SameLine(0.0f, 8.0f);
		ImGui::TextDisabled("%s", name.c_str());
	}
};

template<FixedString Id>
struct Selectable {
	static constexpr auto id = Id;

	std::string name;
	bool* selected = nullptr;

	Size hint(const Size& avail) const {
		const float h = ImGui::CalcTextSize(name.c_str()).y + ImGui::GetStyle().FramePadding.y * 2.0f;
		return Size{avail.w > 0.0f ? avail.w : 160.0f, h};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		if (!selected) return;
		const ImVec2 sp = ImGui::GetStyle().ItemSpacing;
		ImGui::SetCursorPos(ImVec2(
			ImGui::GetCursorPos().x + std::trunc(sp.x * 0.5f),
			ImGui::GetCursorPos().y + std::trunc(sp.y * 0.5f)));
		ImGui::Selectable(name.c_str(), selected, 0, ImVec2(r.w - sp.x, r.h - sp.y));
	}
};

template<FixedString Id>
struct SeparatorText {
	static constexpr auto id = Id;

	std::string name;

	Size hint(const Size&) const {
		return Size{0.0f, ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect&) {
		ImGui::SeparatorText(name.c_str());
	}
};

// 等比放进盒子的矩形（居中）
struct FitRect { float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f; };
inline FitRect fitRect(float srcW, float srcH, float boxW, float boxH) {
	FitRect r{0.0f, 0.0f, boxW, boxH};
	if (srcW <= 0.0f || srcH <= 0.0f || boxW <= 0.0f || boxH <= 0.0f) return r;
	const float scale = std::min(boxW / srcW, boxH / srcH);
	r.w = srcW * scale;
	r.h = srcH * scale;
	r.x = (boxW - r.w) * 0.5f;
	r.y = (boxH - r.h) * 0.5f;
	return r;
}

template<FixedString Id>
struct Image {
	static constexpr auto id = Id;

	ImTextureID tex{};
	ImVec2 size{64.0f, 64.0f};
	ImVec2 srcSize{0.0f, 0.0f};      // 原图尺寸（keepAspect 时用）
	bool keepAspect = false;         // 在布局矩形内等比缩放（居中）
	float rounding = 0.0f;           // 圆角
	float border = 0.0f;             // 边框粗细
	ImU32 borderColor = 0;           // 0 = 主题边框色
	ImU32 tint = 0;                  // 0 = 原色
	ImVec2 uv0{0.0f, 0.0f}, uv1{1.0f, 1.0f};

	Size hint(const Size&) const { return Size{size.x, size.y}; }
	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		float w = size.x;
		float h = size.y;
		if (keepAspect && r.w > 0.0f && r.h > 0.0f) { w = r.w; h = r.h; }
		if (w <= 0.0f || h <= 0.0f) return;

		ImVec2 o(0.0f, 0.0f);
		ImVec2 e(w, h);
		if (keepAspect && srcSize.x > 0.0f && srcSize.y > 0.0f) {
			const FitRect fit = fitRect(srcSize.x, srcSize.y, w, h);
			o = ImVec2(fit.x, fit.y);
			e = ImVec2(fit.x + fit.w, fit.y + fit.h);
		}

		ImGui::SetCursorPos(ImVec2(r.x, r.y));
		const ImVec2 sp = ImGui::GetCursorScreenPos();
		const ImVec2 a(sp.x + o.x, sp.y + o.y);
		const ImVec2 b(sp.x + e.x, sp.y + e.y);
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const ImTextureID t = textureOr(tex);   // 未设纹理 → 引擎默认棋盘
		const ImU32 col = tint ? tint : IM_COL32_WHITE;
		if (rounding > 0.0f) dl->AddImageRounded(t, a, b, uv0, uv1, col, rounding);
		else dl->AddImage(t, a, b, uv0, uv1, col);
		if (border > 0.0f) {
			dl->AddRect(a, b, borderColor ? borderColor : ImGui::GetColorU32(ImGuiCol_Border),
				rounding, 0, border);
		}
	}
};

template<FixedString Id>
struct ImageButton {
	static constexpr auto id = Id;

	ImTextureID tex{};
	ImVec2 size{64.0f, 64.0f};
	std::function<void()> onClick;

	template<class F>
	void setOnClick(F&& cb) { onClick = std::forward<F>(cb); }

	Size hint(const Size&) const { return Size{size.x, size.y}; }
	Policy policy() const { return Policy{}; }

	void draw(const Rect&) {
		if (ImGui::ImageButton("wg2_imagebtn", textureOr(tex), size) && onClick) onClick();
	}
};

template<FixedString Id>
struct PlotLines {
	static constexpr auto id = Id;

	std::string name;
	const std::vector<float>* values = nullptr;
	// 拉取式数据：设置后每帧取一次（自行生成曲线，避免外部维护缓冲）
	std::function<const std::vector<float>&()> valuesFn;
	ImVec2 size{0.0f, 60.0f};

	const std::vector<float>* data() const { return valuesFn ? &valuesFn() : values; }

	Size hint(const Size& avail) const {
		return Size{size.x > 0.0f ? size.x : avail.w, size.y};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		const std::vector<float>* values = data();
		if (!values || values->empty()) return;
		const float maxf = std::numeric_limits<float>::max();
		ImGui::PlotLines(name.c_str(), values->data(), static_cast<int>(values->size()),
			0, nullptr, maxf, maxf,
			ImVec2(detail::frameWidthFor(r.w, name.c_str()), r.h));
	}
};


template<FixedString Id>
struct EllipsisLabel {
	static constexpr auto id = Id;

	std::string text;
	std::function<std::string()> valueFn;

	template<class F>
	void setValueFn(F&& fn) { valueFn = std::forward<F>(fn); }

	Size hint(const Size& avail) const {
		const std::string shown = valueFn ? valueFn() : text;
		const ImVec2 s = ImGui::CalcTextSize(shown.c_str());
		return Size{avail.w > 0.0f ? avail.w : s.x, s.y};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		const std::string shown = valueFn ? valueFn() : text;
		ImGui::TextUnformatted(detail::ellipsize(shown, std::max(r.w, 0.0f)).c_str());
	}
};

} // namespace Mk::wg2
