#pragma once
// 导航 + 页面宿主（框架层）
//   NavTree  : 侧栏导航树（WinUI NavigationView 风格）：分组节点可展开/收起，叶子节点就是页
//   PageHost : 按页序号渲染一页（类型擦除的渲染回调）
//   PageBook : 用布局容器 HBox<NavTree, PageHost> 组合页簿——宽度切分交给布局引擎
//              （导航固定 em 宽度，宿主 stretch 吃剩余），页簿只下发/收回选中页。
//   选中页归 PageBook 所有；分组展开状态归导航控件自己（控件数据，不经过引擎登记）。
#include "../core.h"
#include "../static_layout.h"

#include <imgui.h>

#include <algorithm>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace Mk::wg2 {

namespace detail {

// 按可用宽度截断并加省略号（UTF-8 安全，按当前字体度量）
inline std::string ellipsizeToWidth(const char* text, float avail) {
	std::string s(text ? text : "");
	if (avail <= 0.0f || ImGui::CalcTextSize(s.c_str()).x <= avail) return s;
	const float ellW = ImGui::CalcTextSize("...").x;
	std::string out;
	for (std::size_t i = 0; i < s.size();) {
		const unsigned char c = static_cast<unsigned char>(s[i]);
		const std::size_t len = c < 0x80 ? 1 : (c >> 5) == 0x06 ? 2 : (c >> 4) == 0x0E ? 3 : 4;
		std::string next = out + s.substr(i, len);
		if (ImGui::CalcTextSize(next.c_str()).x + ellW > avail) break;
		out = std::move(next);
		i += len;
	}
	return out + "...";
}

} // namespace detail

// ---------- 导航树 ----------

struct NavNode {
	std::string label;
	bool group = false;        // true = 分组节点（可展开），false = 页
	bool expanded = true;      // 分组的展开状态
	std::vector<NavNode> children;
};

template<FixedString Id>
struct NavTree {
	static constexpr auto id = Id;

	std::vector<NavNode> roots;
	int selected = 0;                // 选中页序号（DFS 叶子顺序）：页簿每帧写入，点击时回写
	std::vector<std::size_t> open_;  // 建树时的打开路径（下标链；不存指针，避免扩容失效）

	// —— 建树：声明顺序即 DFS 叶子顺序 ——
	// addGroup 开一个顶层分组（自动结束上一个顶层分组），其后的 addPage 进入该组
	void addGroup(std::string title) {
		open_.clear();
		roots.push_back(NavNode{std::move(title), true, true, {}});
		open_.push_back(roots.size() - 1);
	}
	// 在当前分组里再开一个子分组（可嵌套）
	void beginGroup(std::string title) {
		auto& kids = currentChildren();
		kids.push_back(NavNode{std::move(title), true, true, {}});
		open_.push_back(kids.size() - 1);
	}
	void endGroup() {
		if (!open_.empty()) open_.pop_back();
	}
	// 叶子节点（页）：放进当前分组，没有分组时挂在根上
	void addPage(std::string title) {
		currentChildren().push_back(NavNode{std::move(title), false, true, {}});
	}

	int pageCount() const { return countLeaves(roots); }

	// 第 page 个叶子的文本；越界返回空串
	const char* pageTitle(int page) const {
		int idx = 0;
		const char* hit = titleAt(roots, page, idx);
		return hit ? hit : "";
	}

	// 可见行数（收起的分组不显示子项）
	int visibleRows() const { return countVisible(roots); }

	Size hint(const Size& avail) const {
		const float rowH = ImGui::GetFrameHeight();
		return Size{avail.w > 0.0f ? avail.w : 12.0f * ImGui::GetFontSize(),
			rowH * static_cast<float>(visibleRows())};
	}

	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		// BeginChild 即使返回 false 也必须配对 EndChild
		ImGui::SetCursorPos(ImVec2(r.x, r.y));   // 按矩形定位（容器给窗口局部坐标）
		// 侧栏：比内容区略深一档的底色 + 内边距，形成清晰的层级
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
		const bool open = ImGui::BeginChild("wg2_navtree", ImVec2(r.w, r.h), ImGuiChildFlags_Border);
		if (open) {
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 4.0f));
			int leaf = 0;
			drawNodes(roots, 0, leaf);
			ImGui::PopStyleVar();
		}
		ImGui::EndChild();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
	}

private:
	std::vector<NavNode>& currentChildren() {
		std::vector<NavNode>* kids = &roots;
		for (const std::size_t i : open_) {
			if (i >= kids->size()) return roots;   // 兜底：路径失效就回根
			kids = &(*kids)[i].children;
		}
		return *kids;
	}

	static int countLeaves(const std::vector<NavNode>& nodes) {
		int n = 0;
		for (const auto& node : nodes) n += node.group ? countLeaves(node.children) : 1;
		return n;
	}

	static int countVisible(const std::vector<NavNode>& nodes) {
		int n = 0;
		for (const auto& node : nodes) {
			++n;
			if (node.group && node.expanded) n += countVisible(node.children);
		}
		return n;
	}

	static const char* titleAt(const std::vector<NavNode>& nodes, int want, int& idx) {
		for (const auto& node : nodes) {
			if (node.group) {
				if (const char* hit = titleAt(node.children, want, idx)) return hit;
			} else {
				if (idx++ == want) return node.label.c_str();
			}
		}
		return nullptr;
	}

	// 一行节点：分组画箭头 + 次要色文字；叶子画选中强调条 + 正文色
	void drawNodes(std::vector<NavNode>& nodes, int depth, int& leaf) {
		const ImGuiStyle& st = ImGui::GetStyle();
		const float rowH = ImGui::GetFrameHeight();
		const float indentStep = st.IndentSpacing * 0.62f;
		ImDrawList* dl = ImGui::GetWindowDrawList();

		bool first = true;
		for (auto& node : nodes) {
			if (node.group && depth == 0 && !first) ImGui::Dummy(ImVec2(0.0f, st.ItemSpacing.y * 1.5f));
			first = false;
			ImGui::PushID(static_cast<const void*>(&node));
			if (depth > 0) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + static_cast<float>(depth) * indentStep);

			const bool isLeaf = !node.group;
			const bool sel = isLeaf && leaf == selected;
			const float rowW = ImGui::GetContentRegionAvail().x;
			const bool pressed = ImGui::Selectable("##row", sel, ImGuiSelectableFlags_None, ImVec2(rowW, rowH));

			const ImVec2 rmin = ImGui::GetItemRectMin();
			const ImVec2 rmax = ImGui::GetItemRectMax();
			const float cy = 0.5f * (rmin.y + rmax.y);
			float textX = rmin.x + st.FramePadding.x;

			if (node.group) {
				// 展开箭头：两笔线画出来，和字体无关（▸ / ▾）
				const float a = ImGui::GetFontSize() * 0.20f;
				const ImU32 arrow = ImGui::GetColorU32(ImGuiCol_Text, 0.80f);
				if (node.expanded) {
					dl->AddLine(ImVec2(textX - a, cy - a * 0.55f), ImVec2(textX, cy + a * 0.45f), arrow, 1.0f);
					dl->AddLine(ImVec2(textX + a, cy - a * 0.55f), ImVec2(textX, cy + a * 0.45f), arrow, 1.0f);
				} else {
					dl->AddLine(ImVec2(textX - a * 0.55f, cy - a), ImVec2(textX + a * 0.45f, cy), arrow, 1.0f);
					dl->AddLine(ImVec2(textX - a * 0.55f, cy + a), ImVec2(textX + a * 0.45f, cy), arrow, 1.0f);
				}
				textX += a * 2.0f + st.ItemInnerSpacing.x;
				const float avail = rmax.x - textX - st.FramePadding.x;
				const std::string clipped = detail::ellipsizeToWidth(node.label.c_str(), avail);
				dl->AddText(ImVec2(textX, cy - ImGui::GetTextLineHeight() * 0.5f),
					ImGui::GetColorU32(ImGuiCol_TextDisabled), clipped.c_str());
				if (pressed) node.expanded = !node.expanded;
			} else {
				textX += indentStep * 0.25f;
				const float avail = rmax.x - textX - st.FramePadding.x;
				const std::string clipped = detail::ellipsizeToWidth(node.label.c_str(), avail);
				dl->AddText(ImVec2(textX, cy - ImGui::GetTextLineHeight() * 0.5f),
					sel ? ImGui::GetColorU32(ImGuiCol_CheckMark) : ImGui::GetColorU32(ImGuiCol_Text), clipped.c_str());
				if (sel) {
					// WinUI 式左侧强调条（贴在行首）
					const float barW = std::max(2.0f, ImGui::GetFontSize() * 0.16f);
					const float pad = st.FramePadding.y * 0.5f;
					dl->AddRectFilled(ImVec2(rmin.x + 1.0f, rmin.y + pad),
						ImVec2(rmin.x + 1.0f + barW, rmax.y - pad),
						ImGui::GetColorU32(ImGuiCol_CheckMark), barW * 0.5f);
				}
				if (pressed) selected = leaf;
			}

			ImGui::PopID();

			if (node.group) {
				if (node.expanded) drawNodes(node.children, depth + 1, leaf);
				else leaf += countLeaves(node.children);   // 收起也占页序号，页面索引保持稳定
			} else {
				++leaf;
			}
		}
	}
};

// ---------- 页面宿主 ----------

template<FixedString Id>
struct PageHost {
	static constexpr auto id = Id;

	int page = 0;                    // 要渲染的页序号（独立使用时）
	const int* selected = nullptr;   // 跟随某个选中页（页簿接线：与导航同帧切换）
	std::vector<std::function<void(const Rect&)>> pages;

	template<class F>
	void addPage(F&& render) { pages.push_back(std::forward<F>(render)); }

	int pageCount() const { return static_cast<int>(pages.size()); }

	Size hint(const Size& avail) const { return Size{avail.w, avail.h}; }
	Policy policy() const { return Policy{}; }

	// 页面自己负责滚动层；宿主只把矩形交给当前页
	void draw(const Rect& r) {
		const int p = selected ? *selected : page;
		const int cnt = pageCount();
		if (p >= 0 && p < cnt && pages[p]) pages[p](r);
	}
};

// 页脚占位：没有页脚时保持零高度（默认）
struct EmptyFooter {
	static constexpr auto id = Mk::wg2::Id("footer");
	Size hint(const Size&) const { return Size{0.0f, 0.0f}; }
	Policy policy() const { return Policy{}; }
	void draw(const Rect&) {}
};

// 侧栏宽度（字号倍数）
inline constexpr float kNavWidthEm = 10.0f;

// ---------- 页簿：导航树 + 页面宿主，用布局容器合成 ----------
//   宽度切分交给 HBox：导航固定 em 宽度，宿主 stretch 吃掉剩余；
//   页簿只做两件事——下发选中页、收回点击。

template<FixedString Id, class Footer = EmptyFooter>
struct PageBook {
	static constexpr auto id = Id;

	using Nav  = NavTree<Mk::wg2::Id("nav")>;
	using Host = PageHost<Mk::wg2::Id("host")>;
	using Root = HBox<Id, Nav, VBox<Mk::wg2::Id("main"), Host, Footer>>;   // 导航 +（页面 + 页脚）

	static constexpr float kNavWidthEm = Mk::wg2::kNavWidthEm;

	Root root;
	int current = 0;              // 当前页序号（叶子顺序，不含分组）

	PageBook() {
		auto& n = nav();
		auto& h = host();
		h.selected = &n.selected;                     // 宿主跟随导航选中页：点击后同帧换页
		Mk::wg2::sizeOf(n).fixedWEm = kNavWidthEm;    // 导航宽度按字号
		auto& m = root.at<Mk::wg2::Id("main")>();
		Mk::wg2::setStretch(m, 1.0f);                 // 右列吃掉剩余宽度
		Mk::wg2::setStretch(h, 1.0f);                 // 页面吃掉右列剩余高度
		root.marginX = 0.0f;                         // 整块内容区自己贴边
		root.marginY = 0.0f;
	}

	Nav& nav() { return root.at<Mk::wg2::Id("nav")>(); }
	const Nav& nav() const { return root.at<Mk::wg2::Id("nav")>(); }
	Host& host() { return root.at<Mk::wg2::Id("main"), Mk::wg2::Id("host")>(); }
	const Host& host() const { return root.at<Mk::wg2::Id("main"), Mk::wg2::Id("host")>(); }
	Footer& footer() { return root.at<Mk::wg2::Id("main"), Footer::id>(); }
	const Footer& footer() const { return root.at<Mk::wg2::Id("main"), Footer::id>(); }

	template<class F>
	void addPage(std::string title, F&& render) {
		nav().addPage(std::move(title));
		host().addPage(std::forward<F>(render));
	}
	void addGroup(std::string title) { nav().addGroup(std::move(title)); }
	void beginGroup(std::string title) { nav().beginGroup(std::move(title)); }
	void endGroup() { nav().endGroup(); }

	int pageCount() const { return host().pageCount(); }

	// 选中页：越界夹到合法范围（无页时归 0）
	void select(int page) {
		const int n = pageCount();
		current = n > 0 ? std::clamp(page, 0, n - 1) : 0;
	}
	const char* currentTitle() const { return nav().pageTitle(current); }

	Size hint(const Size& avail) const { return Size{avail.w, avail.h}; }
	Policy policy() const { return policyOf(root); }

	void draw(const Rect& r) {
		nav().selected = current;                     // 选中页归页簿：导航只显示
		host().page = current;
		Mk::wg2::layout(root, r);                     // 布局容器统一切分宽度
		current = nav().selected;                     // 点击导航：收回选中页
		host().page = current;
	}
};

} // namespace Mk::wg2
