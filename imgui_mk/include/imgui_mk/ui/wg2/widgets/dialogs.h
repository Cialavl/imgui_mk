#pragma once
// Mk::wg2 对话框族：确认框 / Toast / 加载指示器
#include <algorithm>
#include <cmath>
#include <functional>
#include <string>
#include <vector>

#include <imgui.h>
#include <imgui_mk/ui/wg2/core.h>
#include <imgui_mk/ui/wg2/widget.h>

namespace Mk::wg2 {

// ============ 确认框（模态） ============
struct ConfirmDialog {
	static constexpr auto id = Id("confirm");

	std::string idname = "wg2_confirm";
	std::string title = "确认";
	std::string text;
	std::string okLabel = "确定";
	std::string cancelLabel = "取消";
	bool open = false;
	std::function<void()> onOk;
	std::function<void()> onCancel;

	void show(std::string t, std::string body) {
		title = std::move(t);
		text = std::move(body);
		open = true;
	}

	Size hint(const Size&) const { return Size{0.0f, 0.0f}; }
	Policy policy() const { return Policy{}; }

	void draw(const Rect&) {
		if (open) {
			ImGui::OpenPopup(idname.c_str());
			open = false;
		}
		ImGui::SetNextWindowSize(ImVec2(360.0f, 0.0f), ImGuiCond_Appearing);
		if (!ImGui::BeginPopupModal(idname.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;
		ImGui::TextUnformatted(title.c_str());
		ImGui::Separator();
		ImGui::TextWrapped("%s", text.c_str());
		ImGui::Spacing();
		if (ImGui::Button(okLabel.c_str(), ImVec2(96.0f, 0.0f))) {
			if (onOk) onOk();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button(cancelLabel.c_str(), ImVec2(96.0f, 0.0f))) {
			if (onCancel) onCancel();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
};

// ============ Toast（右下角堆叠，自动过期） ============
struct ToastHost {
	static constexpr auto id = Id("toasts");

	struct Item {
		std::string text;
		float ttl = 3.0f;
		float life = 0.0f;
	};
	std::vector<Item> items;

	void toast(std::string text, float seconds = 3.0f) {
		items.push_back(Item{std::move(text), seconds > 0.0f ? seconds : 0.1f, 0.0f});
	}

	// 推进生命周期（draw 里用帧 dt；测试可直接调）
	void advance(float dt) {
		for (auto& it : items) it.life += dt;
		items.erase(std::remove_if(items.begin(), items.end(),
			[](const Item& it) { return it.life >= it.ttl; }), items.end());
	}

	Size hint(const Size&) const { return Size{0.0f, 0.0f}; }
	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		advance(ImGui::GetIO().DeltaTime);
		if (items.empty()) return;
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const float pad = 10.0f;
		float y = r.y + r.h - pad;
		for (auto it = items.rbegin(); it != items.rend(); ++it) {
			const float remain = it->ttl - it->life;
			const float alpha = std::min(1.0f, remain / 0.4f);   // 最后 0.4s 淡出
			const ImVec2 ts = ImGui::CalcTextSize(it->text.c_str());
			const float w = ts.x + pad * 2.0f;
			const float h = ts.y + pad;
			const ImVec2 p0(r.x + r.w - w - pad, y - h);
			const ImVec2 p1(p0.x + w, y);
			dl->AddRectFilled(p0, p1, ImGui::GetColorU32(ImGuiCol_PopupBg, alpha), 6.0f);
			dl->AddRect(p0, p1, ImGui::GetColorU32(ImGuiCol_Border, alpha), 6.0f);
			dl->AddText(ImVec2(p0.x + pad, p0.y + pad * 0.5f),
				ImGui::GetColorU32(ImGuiCol_Text, alpha), it->text.c_str());
			y -= h + 6.0f;
		}
	}
};

// ============ 加载指示器（不确定进度） ============
template<FixedString Id>
struct Spinner {
	static constexpr auto id = Id;

	float radius = 8.0f;
	float thickness = 2.0f;
	ImU32 color = 0;   // 0 = 主题文字色

	Size hint(const Size&) const { return Size{radius * 2.0f + 4.0f, radius * 2.0f + 4.0f}; }
	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const ImVec2 c(r.x + r.w * 0.5f, r.y + r.h * 0.5f);
		const ImU32 col = color ? color : ImGui::GetColorU32(ImGuiCol_Text);
		const float t = static_cast<float>(ImGui::GetTime());
		const float a0 = std::fmod(t * 6.0f, 6.2831853f);
		dl->PathClear();
		constexpr int kSeg = 24;
		for (int i = 0; i <= kSeg; ++i) {
			const float a = a0 + static_cast<float>(i) / kSeg * 3.6f;
			dl->PathLineTo(ImVec2(c.x + std::cos(a) * radius, c.y + std::sin(a) * radius));
		}
		dl->PathStroke(col, 0, thickness);
	}
};

} // namespace Mk::wg2
