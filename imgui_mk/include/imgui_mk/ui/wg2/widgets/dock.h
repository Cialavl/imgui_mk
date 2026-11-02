#pragma once
// Mk::wg2 停靠布局（DockSpace）：分割树 + 页签组 + 拖拽停靠 + 可拖分隔条
//
//   结构：节点池中的二叉树，叶子是页签组（多个面板共享一个页签栏），内部节点是横/纵分割。
//   交互：拖页签到别的页签栏 = 合并到该组；拖到叶子四边 = 分割停靠；拖分隔条 = 调整比例。
//   持久化：serialize/deserialize 保存树结构与面板归属（面板按 addPanel 顺序编号）。
//
//   DockSpace dock;
//   int viewport = dock.addPanel("视口", [](const Rect& r){ ... });
//   int inspector = dock.addPanel("属性", [](const Rect& r){ ... });
//   dock.dockPanel(inspector, dock.root(), DockEdge::Right);
//   // 树里放：DockSpace 控件（draw 由布局驱动）
#include <algorithm>
#include <cmath>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

#include <imgui.h>
#include <imgui_mk/ui/wg2/core.h>
#include <imgui_mk/ui/wg2/widget.h>
#include <imgui_mk/ui/wg2/widgets/dragdrop.h>

namespace Mk::wg2 {

enum class DockAxis { Horizontal, Vertical };   // Horizontal: 左右分；Vertical: 上下分
enum class DockEdge { Center, Left, Right, Top, Bottom };

class DockSpace {
public:
	struct Panel {
		std::string title;
		std::function<void(const Rect&)> draw;
	};

	// —— 面板 ——
	int addPanel(std::string title, std::function<void(const Rect&)> draw) {
		panels_.push_back(Panel{std::move(title), std::move(draw)});
		const int id = static_cast<int>(panels_.size()) - 1;
		nodes_[root_].panels.push_back(id);
		return id;
	}

	int panelCount() const { return static_cast<int>(panels_.size()); }
	const Panel* panel(int id) const {
		return (id >= 0 && id < static_cast<int>(panels_.size())) ? &panels_[static_cast<std::size_t>(id)] : nullptr;
	}

	// —— 结构查询 ——
	int root() const { return root_; }
	int nodeCount() const { return static_cast<int>(nodes_.size()); }
	bool isLeaf(int node) const { return valid(node) && nodes_[static_cast<std::size_t>(node)].leaf; }
	int leafOf(int panelId) const {
		for (int i = 0; i < nodeCount(); ++i) {
			const auto& n = nodes_[static_cast<std::size_t>(i)];
			if (!n.leaf) continue;
			if (std::find(n.panels.begin(), n.panels.end(), panelId) != n.panels.end()) return i;
		}
		return -1;
	}
	std::vector<int> leafPanels(int node) const {
		return isLeaf(node) ? nodes_[static_cast<std::size_t>(node)].panels : std::vector<int>{};
	}
	int leafCount() const {
		int n = 0;
		for (const auto& node : nodes_) if (node.leaf) ++n;
		return n;
	}

	// —— 结构操作 ——
	// 移动面板到目标叶子的页签组（Center 语义）
	bool movePanel(int panelId, int targetLeaf) {
		if (!isLeaf(targetLeaf)) return false;
		const int src = leafOf(panelId);
		if (src < 0) return false;
		if (src == targetLeaf) return true;
		auto& sp = nodes_[static_cast<std::size_t>(src)].panels;
		sp.erase(std::remove(sp.begin(), sp.end(), panelId), sp.end());
		auto& sn = nodes_[static_cast<std::size_t>(src)];
		sn.active = std::clamp(sn.active, 0, std::max(0, static_cast<int>(sp.size()) - 1));
		auto& tn = nodes_[static_cast<std::size_t>(targetLeaf)];
		tn.panels.push_back(panelId);
		tn.active = static_cast<int>(tn.panels.size()) - 1;
		return true;
	}

	// 停靠：Center = 并入页签组；四边 = 把目标叶子拆开，新面板占一侧
	bool dockPanel(int panelId, int targetLeaf, DockEdge edge) {
		if (!isLeaf(targetLeaf) || leafOf(panelId) < 0) return false;
		if (edge == DockEdge::Center) return movePanel(panelId, targetLeaf);

		const int src = leafOf(panelId);
		auto& sp = nodes_[static_cast<std::size_t>(src)].panels;
		sp.erase(std::remove(sp.begin(), sp.end(), panelId), sp.end());

		// 先取现有页签（拷贝），再分配新节点——分配会让 nodes_ 重新分配，不能提前持有引用
		std::vector<int> existing = nodes_[static_cast<std::size_t>(targetLeaf)].panels;
		existing.erase(std::remove(existing.begin(), existing.end(), panelId), existing.end());

		const bool vertical = (edge == DockEdge::Top || edge == DockEdge::Bottom);
		const int a = newNode(true);
		const int b = newNode(true);
		const bool panelFirst = (edge == DockEdge::Left || edge == DockEdge::Top);
		if (panelFirst) {
			nodes_[static_cast<std::size_t>(a)].panels = {panelId};
			nodes_[static_cast<std::size_t>(b)].panels = std::move(existing);
		}
		else {
			nodes_[static_cast<std::size_t>(a)].panels = std::move(existing);
			nodes_[static_cast<std::size_t>(b)].panels = {panelId};
		}
		auto& target = nodes_[static_cast<std::size_t>(targetLeaf)];
		target.panels.clear();
		target.active = 0;
		target.leaf = false;
		target.vertical = vertical;
		target.ratio = 0.5f;
		target.a = a;
		target.b = b;
		return true;
	}

	// 拆分叶子：原有页签归一侧，另一侧为空（结构/测试用）
	bool split(int leaf, DockAxis axis, float ratio = 0.5f) {
		if (!isLeaf(leaf)) return false;
		const std::vector<int> existing = nodes_[static_cast<std::size_t>(leaf)].panels;
		const int a = newNode(true);
		const int b = newNode(true);
		nodes_[static_cast<std::size_t>(a)].panels = existing;
		auto& n = nodes_[static_cast<std::size_t>(leaf)];
		n.panels.clear();
		n.leaf = false;
		n.vertical = (axis == DockAxis::Vertical);
		n.ratio = std::clamp(ratio, 0.1f, 0.9f);
		n.a = a;
		n.b = b;
		return true;
	}

	// —— 持久化 ——
	std::string serialize() const {
		std::ostringstream os;
		os << "DOCK 1 " << nodes_.size() << ' ' << root_ << '\n';
		for (const auto& n : nodes_) {
			if (n.leaf) {
				os << "L " << n.panels.size() << ' ' << n.active;
				for (int id : n.panels) os << ' ' << id;
				os << '\n';
			}
			else {
				os << "S " << (n.vertical ? 1 : 0) << ' ' << n.ratio << ' ' << n.a << ' ' << n.b << '\n';
			}
		}
		return os.str();
	}

	bool deserialize(const std::string& text) {
		std::istringstream is(text);
		std::string tag;
		int version = 0;
		std::size_t count = 0;
		int root = 0;
		if (!(is >> tag >> version >> count >> root) || tag != "DOCK" || version != 1) return false;
		if (count == 0 || count > 4096) return false;

		std::vector<Node> nodes(count);
		for (std::size_t i = 0; i < count; ++i) {
			char kind = 0;
			if (!(is >> kind)) return false;
			if (kind == 'L') {
				std::size_t n = 0;
				int active = 0;
				if (!(is >> n >> active) || n > 256) return false;
				nodes[i].leaf = true;
				nodes[i].active = active;
				nodes[i].panels.resize(n);
				for (std::size_t k = 0; k < n; ++k) {
					if (!(is >> nodes[i].panels[k])) return false;
				}
			}
			else if (kind == 'S') {
				int vertical = 0;
				float ratio = 0.5f;
				int a = -1;
				int b = -1;
				if (!(is >> vertical >> ratio >> a >> b)) return false;
				nodes[i].leaf = false;
				nodes[i].vertical = vertical != 0;
				nodes[i].ratio = ratio;
				nodes[i].a = a;
				nodes[i].b = b;
			}
			else {
				return false;
			}
		}
		if (root < 0 || root >= static_cast<int>(count)) return false;
		nodes_ = std::move(nodes);
		root_ = root;
		return true;
	}

	// —— 绘制 ——
	Size hint(const Size& avail) const {
		return Size{avail.w > 0.0f ? avail.w : 720.0f, avail.h > 0.0f ? avail.h : 480.0f};
	}
	Policy policy() const { return Policy{}; }

	void draw(const Rect& r) {
		if (r.w <= 4.0f || r.h <= 4.0f) return;
		ImGui::SetCursorPos(ImVec2(r.x, r.y));
		const ImVec2 sp = ImGui::GetCursorScreenPos();
		// 不超出窗口可视区（父级可用宽度可能因滚动内容而偏大）
		const ImGuiViewport* vp = ImGui::GetMainViewport();
		float w = r.w;
		float h = r.h;
		if (vp) {
			w = std::min(w, vp->Pos.x + vp->Size.x - sp.x);
			h = std::min(h, vp->Pos.y + vp->Size.y - sp.y);
		}
		if (w <= 4.0f || h <= 4.0f) return;

		ImGui::PushID(this);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		if (ImGui::BeginChild("##dock", ImVec2(w, h), 0,
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
			pending_.clear();
			drawNode(root_, Rect{0.0f, 0.0f, w, h});
			for (const auto& p : pending_) dockPanel(p.panel, p.leaf, p.edge);
			pending_.clear();
		}
		ImGui::EndChild();
		ImGui::PopStyleVar();
		ImGui::PopID();
	}

private:
	struct Node {
		bool leaf = true;
		std::vector<int> panels;
		int active = 0;
		bool vertical = false;
		float ratio = 0.5f;
		int a = -1;
		int b = -1;
	};

	struct Pending {
		int panel = -1;
		int leaf = -1;
		DockEdge edge = DockEdge::Center;
	};

	bool valid(int node) const { return node >= 0 && node < nodeCount(); }

	int newNode(bool leaf) {
		Node n;
		n.leaf = leaf;
		nodes_.push_back(std::move(n));
		return static_cast<int>(nodes_.size()) - 1;
	}

	void drawNode(int idx, const Rect& local) {
		if (!valid(idx) || local.w <= 1.0f || local.h <= 1.0f) return;
		const Node n = nodes_[static_cast<std::size_t>(idx)];   // 值拷贝：递归中不持有引用
		if (n.leaf) {
			drawLeaf(idx, n, local);
			return;
		}
		constexpr float gap = 6.0f;
		if (!n.vertical) {
			const float usable = std::max(1.0f, local.w - gap);
			const float wa = usable * std::clamp(n.ratio, 0.1f, 0.9f);
			drawNode(n.a, Rect{local.x, local.y, wa, local.h});
			drawSplitter(idx, Rect{local.x + wa, local.y, gap, local.h}, true, usable);
			drawNode(n.b, Rect{local.x + wa + gap, local.y, local.w - wa - gap, local.h});
		}
		else {
			const float usable = std::max(1.0f, local.h - gap);
			const float ha = usable * std::clamp(n.ratio, 0.1f, 0.9f);
			drawNode(n.a, Rect{local.x, local.y, local.w, ha});
			drawSplitter(idx, Rect{local.x, local.y + ha, local.w, gap}, false, usable);
			drawNode(n.b, Rect{local.x, local.y + ha + gap, local.w, local.h - ha - gap});
		}
	}

	void drawSplitter(int nodeIdx, const Rect& local, bool horizontal, float usable) {
		ImGui::SetCursorPos(ImVec2(local.x, local.y));
		ImGui::PushID(nodeIdx);
		ImGui::InvisibleButton("##split", ImVec2(std::max(1.0f, local.w), std::max(1.0f, local.h)));
		if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
			ImGui::SetMouseCursor(horizontal ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS);
		}
		if (ImGui::IsItemActive()) {
			const ImVec2 d = ImGui::GetIO().MouseDelta;
			float& ratio = nodes_[static_cast<std::size_t>(nodeIdx)].ratio;
			ratio = std::clamp(ratio + (horizontal ? d.x : d.y) / usable, 0.1f, 0.9f);
		}
		ImGui::PopID();
	}

	void drawLeaf(int idx, const Node& n, const Rect& local) {
		const float barH = ImGui::GetFrameHeight();
		const ImGuiStyle& style = ImGui::GetStyle();
		const ImU32 barBg = ImGui::GetColorU32(ImGuiCol_MenuBarBg);
		ImGui::SetCursorPos(ImVec2(local.x, local.y));
		const ImVec2 barMin = ImGui::GetCursorScreenPos();
		ImGui::GetWindowDrawList()->AddRectFilled(barMin, ImVec2(barMin.x + local.w, barMin.y + barH), barBg);

		// 页签栏
		float x = local.x;
		const int active = n.panels.empty() ? -1 : std::clamp(n.active, 0, static_cast<int>(n.panels.size()) - 1);
		for (int id : n.panels) {
			const std::string title = panels_[static_cast<std::size_t>(id)].title;
			const float tw = ImGui::CalcTextSize(title.c_str()).x + style.FramePadding.x * 2.0f + 16.0f;
			ImGui::SetCursorPos(ImVec2(x, local.y));
			ImGui::PushID(id);
			if (ImGui::Selectable(title.c_str(), id == active, 0, ImVec2(tw, barH))) {
				nodes_[static_cast<std::size_t>(idx)].active = static_cast<int>(
					std::find(n.panels.begin(), n.panels.end(), id) - n.panels.begin());
			}
			dragSource<int>(title.c_str(), id);   // 拖页签到别处
			ImGui::SameLine(0.0f, 0.0f);
			if (ImGui::SmallButton("x")) {
				auto& np = nodes_[static_cast<std::size_t>(idx)].panels;
				np.erase(std::remove(np.begin(), np.end(), id), np.end());
				nodes_[static_cast<std::size_t>(idx)].active = 0;
				ImGui::PopID();
				break;
			}
			ImGui::PopID();
			x += tw + style.ItemSpacing.x;
		}

		// 内容区：每个面板独立子窗口（多行文本/Label 都在面板内左对齐，且被裁剪）
		const Rect content{local.x, local.y + barH, local.w, std::max(0.0f, local.h - barH)};
		if (active >= 0 && panels_[static_cast<std::size_t>(n.panels[static_cast<std::size_t>(active)])].draw) {
			const int pid = n.panels[static_cast<std::size_t>(active)];
			ImGui::SetCursorPos(ImVec2(content.x, content.y));
			ImGui::PushID(pid);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			if (ImGui::BeginChild("##panel", ImVec2(std::max(1.0f, content.w), std::max(1.0f, content.h)), 0,
					ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
				panels_[static_cast<std::size_t>(pid)].draw(Rect{0.0f, 0.0f, content.w, content.h});
			}
			ImGui::EndChild();
			ImGui::PopStyleVar();
			ImGui::PopID();
		}

		// 拖拽中：叶子四边出现停靠热区
		if (ImGui::GetDragDropPayload()) {
			const float bw = std::min(64.0f, local.w * 0.25f);
			const float bh = std::min(64.0f, local.h * 0.25f);
			// 中心 = 并入本页签组；四边 = 分割停靠（后画的热区优先，边缘覆盖中心）
			const DockEdge edges[5] = {DockEdge::Center, DockEdge::Left, DockEdge::Right, DockEdge::Top, DockEdge::Bottom};
			const Rect zones[5] = {
				Rect{local.x + bw, local.y + bh, std::max(1.0f, local.w - bw * 2.0f), std::max(1.0f, local.h - bh * 2.0f)},
				Rect{local.x, local.y, bw, local.h},
				Rect{local.x + local.w - bw, local.y, bw, local.h},
				Rect{local.x, local.y, local.w, bh},
				Rect{local.x, local.y + local.h - bh, local.w, bh},
			};
			for (int i = 0; i < 5; ++i) {
				ImGui::SetCursorPos(ImVec2(zones[i].x, zones[i].y));
				ImGui::PushID(idx * 16 + i + 8);
				ImGui::InvisibleButton("##dock", ImVec2(std::max(1.0f, zones[i].w), std::max(1.0f, zones[i].h)));
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadType<int>())) {
						if (payload->DataSize == static_cast<int>(sizeof(int))) {
							pending_.push_back(Pending{*static_cast<const int*>(payload->Data), idx, edges[i]});
						}
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::PopID();
			}
		}
	}

	std::vector<Node> nodes_{Node{}};   // 0 = 根（初始为叶子）
	std::vector<Panel> panels_;
	int root_ = 0;
	std::vector<Pending> pending_;
};

} // namespace Mk::wg2
