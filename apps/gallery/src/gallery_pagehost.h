#pragma once
// gallery 专属页面集合控件（Qt 风格：窗口添加页面、宿主持集合、导航持目录）。
// PageHost —— 页面宿主：滚动预览子窗口 + 选中页渲染（自持 std::vector<Node*>）。
// PageNav  —— 页面导航：侧栏目录（自持条目集合 + 连续重复分组头去重）。
#include "gallery_state.h"
#include <imgui_mk/ui/wg_extras.h>
#include <vector>

namespace gallery {

using namespace Mk::wg;

// ============ PageHost：页面宿主（Qt QStackedWidget 类比）============

struct PageHostImpl {
	int* current = nullptr;                 // 当前选中页下标
	std::vector<Node*> pages;               // 页面根集合（addPage 注册）

	// Qt 风格：添加页面
	void addPage(Node& root) { pages.push_back(&root); }

	void renderImpl(Node& n) {
		const ImVec2 sz(n.bounds.w, n.bounds.h);
		if(ImGui::BeginChild("preview", sz, ImGuiChildFlags_Border,
			   ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
			const int cnt = (int)pages.size();
			for(int i = 0; i < cnt; ++i) {
				if(pages[i]) pages[i]->visible = (i == *current);   // 非选中页隐藏（不布局不渲染）
			}
			if(*current >= 0 && *current < cnt && pages[*current]) {
				const ImVec2 cp = ImGui::GetCursorPos();
				const ImVec2 avail = ImGui::GetContentRegionAvail();
				pages[*current]->layout(
					Rect{cp.x, cp.y, avail.x > 0.0f ? avail.x : 0.0f, avail.y > 0.0f ? avail.y : 0.0f});
			}
		}
		ImGui::EndChild();
	}
	void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
};
using PageHost = Control<PageHostImpl, DefaultStyle>;

// ============ PageNav：页面导航（Qt QTabWidget 侧栏类比）============

struct PageEntry { const char* label; bool header; };

struct PageNavImpl {
	std::vector<PageEntry> entries;         // 导航目录（addEntry 注册）
	int* current = nullptr;                 // 当前选中页下标
	float itemH = 22.0f;

	// Qt 风格：添加条目（连续重复分组头自动去重）
	void addEntry(const char* label, bool header) {
		if(header && !entries.empty() && entries.back().header) return;   // 去重相邻分组头
		entries.push_back({label, header});
	}

	void renderImpl(Node& n) {
		if(ImGui::BeginChild("pagenav", ImVec2(n.bounds.w, n.bounds.h), 0)) {
			int idx = 0;
			for(const auto& e : entries) {
				if(e.header) {
					ImGui::SeparatorText(e.label);
					continue;
				}
				ImGui::PushID(idx);
				const bool sel = (idx == *current);
				const float avail = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x * 2.0f;
				std::string clipped;   // 超宽才构造截断缓冲（省每帧分配）
				const char* label = e.label;
				if(ImGui::CalcTextSize(e.label).x > avail) { clipped = detail::ellipsize(e.label, avail); label = clipped.c_str(); }
				if(ImGui::Selectable(label, sel)) *current = idx;
				ImGui::PopID();
				++idx;
			}
		}
		ImGui::EndChild();
	}
	Size sizeHintImpl(Node&) {
		float h = 0.0f;
		for(const auto& e : entries) h += e.header ? 26.0f : itemH;
		return Size{0.0f, h};
	}
	void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
};
using PageNav = Control<PageNavImpl, DefaultStyle>;

} // namespace gallery
