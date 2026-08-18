#pragma once
#include <imgui_mk/ui/wg.h>
#include <cmath>
#include <string>

// Mk::wg 核心控件集：容器（本文件）与原子控件（见下）。
// 每个控件 = 自己的模板 struct（数据+功能函数）经 Control 组合，零继承。
// 布局契约：measureImpl 求期望尺寸（两遍布局第一遍），arrangeImpl 给
// 子节点分配 bounds（第二遍），renderImpl 按 bounds 驱动 ImGui 渲染。

namespace Mk::wg {

	// ============ 容器：VBox（垂直，自上而下，间距=spacing）============

	// 子控件类型经模板参数传入（tuple 编译期组合，模板传入组合的容器形态）
	template<typename... Ts>
	struct VBoxImpl;

	// measure/arrange/render 用成员函数（traits 自动探测）
	// 实现拆分：行为函数写成自由函数模板，ControlTraits 的 ADL 兜底能
	// 找到它们——容器行为与数据分离，数据（spacing）在 Data 里
	template<typename... Ts>
	struct VBoxData {
		float spacing = 8.0f;
	};
	template<typename... Ts>
	struct VBoxImpl;

	template<typename... Ts>
	using VBox = Control<VBoxData<Ts...>, DefaultStyle, Ts...>;

	// 自由函数行为（ADL 兜底路径——容器零成员函数，数据在 Data）
	template<typename... Ts>
	void arrangeImpl(VBoxData<Ts...>& self, Node& n, const Rect& r) {
		float y = r.y;
		for(auto* c : n.children) {
			if(!c) continue;
			Size s = c->measure(Size{r.w, r.h - (y - r.y)});
			c->arrange(Rect{r.x, y, s.w, s.h});
			y += s.h + self.spacing;
		}
	}

	template<typename... Ts>
	Size measureImpl(VBoxData<Ts...>& self, Node& n, const Size& avail) {
		Size total{0, 0};
		for(auto* c : n.children) {
			if(!c) continue;
			Size s = c->measure(avail);
			total.w = std::max(total.w, s.w);
			total.h += s.h;
		}
		if(n.children.size() > 1) total.h += self.spacing * (float)(n.children.size() - 1);
		return total;
	}

	template<typename... Ts>
	void renderImpl(VBoxData<Ts...>&, Node& n) {
		for(auto* c : n.children) {
			if(!c) continue;
			ImGui::SetCursorPos(ImVec2(c->bounds.x, c->bounds.y));
			c->draw();
		}
	}

	// ============ HBox（水平，从左到右）============

	template<typename... Ts>
	struct HBoxData {
		float spacing = 8.0f;
	};

	template<typename... Ts>
	using HBox = Control<HBoxData<Ts...>, DefaultStyle, Ts...>;

	template<typename... Ts>
	Size measureImpl(HBoxData<Ts...>& self, Node& n, const Size& avail) {
		Size total{0, 0};
		for(auto* c : n.children) {
			if(!c) continue;
			Size s = c->measure(avail);
			total.w += s.w;
			total.h = std::max(total.h, s.h);
		}
		if(n.children.size() > 1) total.w += self.spacing * (float)(n.children.size() - 1);
		return total;
	}

	template<typename... Ts>
	void arrangeImpl(HBoxData<Ts...>& self, Node& n, const Rect& r) {
		float x = r.x;
		for(auto* c : n.children) {
			if(!c) continue;
			Size s = c->measure(Size{r.w - (x - r.x), r.h});
			c->arrange(Rect{x, r.y, s.w, s.h});
			x += s.w + self.spacing;
		}
	}

	template<typename... Ts>
	void renderImpl(HBoxData<Ts...>&, Node& n) {
		for(auto* c : n.children) {
			if(!c) continue;
			ImGui::SetCursorPos(ImVec2(c->bounds.x, c->bounds.y));
			c->draw();
		}
	}

	// ============ Group（打包整体，无边框）============

	template<typename... Ts>
	struct GroupData {};
	template<typename... Ts>
	using Group = Control<GroupData<Ts...>, DefaultStyle, Ts...>;

	template<typename... Ts>
	Size measureImpl(GroupData<Ts...>&, Node& n, const Size& avail) {
		Size total{0, 0};
		for(auto* c : n.children) {
			if(!c) continue;
			Size s = c->measure(avail);
			total.w = std::max(total.w, s.w);
			total.h = std::max(total.h, s.h);
		}
		return total;
	}

	template<typename... Ts>
	void arrangeImpl(GroupData<Ts...>&, Node& n, const Rect& r) {
		// 叠加：所有子共享同一区域（重叠布局，如标签+输入同行）
		for(auto* c : n.children) {
			if(!c) continue;
			c->arrange(r);
		}
	}

	template<typename... Ts>
	void renderImpl(GroupData<Ts...>&, Node& n) {
		ImGui::BeginGroup();
		for(auto* c : n.children) {
			if(!c) continue;
			ImGui::SetCursorPos(ImVec2(c->bounds.x, c->bounds.y));
			c->draw();
		}
		ImGui::EndGroup();
	}

	// ============ SubWindow（嵌套子窗口，独立滚动，id/尺寸/边框）============

	template<typename... Ts>
	struct SubWindowData {
		std::string id;
		float width = 0.0f;    // 0 = 填充可用
		float height = 0.0f;   // 0 = 填充可用
		bool border = true;
	};
	template<typename... Ts>
	using SubWindow = Control<SubWindowData<Ts...>, DefaultStyle, Ts...>;

	template<typename... Ts>
	Size measureImpl(SubWindowData<Ts...>& self, Node& n, const Size& avail) {
		float w = self.width > 0 ? self.width : avail.w;
		float h = self.height > 0 ? self.height : avail.h;
		// 子控件测量（内容驱动高度）
		Size inner{0, 0};
		for(auto* c : n.children) {
			if(!c) continue;
			Size s = c->measure(Size{w, h});
			inner.h += s.h;
		}
		return Size{w, h > 0 ? h : inner.h};
	}

	template<typename... Ts>
	void arrangeImpl(SubWindowData<Ts...>& self, Node& n, const Rect& r) {
		float y = r.y;
		for(auto* c : n.children) {
			if(!c) continue;
			Size s = c->measure(Size{r.w, r.h - (y - r.y)});
			c->arrange(Rect{r.x, y, s.w, s.h});
			y += s.h;
		}
	}

	template<typename... Ts>
	void renderImpl(SubWindowData<Ts...>& self, Node& n) {
		const ImVec2 sz(n.bounds.w, n.bounds.h);
		if(ImGui::BeginChild(self.id.c_str(), sz,
			   self.border ? ImGuiChildFlags_Border : 0)) {
			for(auto* c : n.children) {
				if(!c) continue;
				ImGui::SetCursorPos(ImVec2(c->bounds.x - n.bounds.x,
					c->bounds.y - n.bounds.y));
				c->draw();
			}
		}
		ImGui::EndChild();
	}

	// ============ TabBar（标签页，仅激活页渲染）============

	template<typename... Ts>
	struct TabData {
		std::string name;
	};
	template<typename... Ts>
	using Tab = Control<TabData<Ts...>, DefaultStyle, Ts...>;

	template<typename... Ts>
	Size measureImpl(TabData<Ts...>&, Node& n, const Size& avail) {
		Size total{0, 0};
		for(auto* c : n.children) {
			if(!c) continue;
			Size s = c->measure(avail);
			total.w = std::max(total.w, s.w);
			total.h = std::max(total.h, s.h);
		}
		return total;
	}
	template<typename... Ts>
	void arrangeImpl(TabData<Ts...>&, Node& n, const Rect& r) {
		for(auto* c : n.children) {
			if(!c) continue;
			c->arrange(r);
		}
	}
	template<typename... Ts>
	void renderImpl(TabData<Ts...>& self, Node& n) {
		(void)self;
		for(auto* c : n.children) {
			if(!c) continue;
			ImGui::SetCursorPos(ImVec2(c->bounds.x - n.bounds.x,
				c->bounds.y - n.bounds.y));
			c->draw();
		}
	}

	// 标签页：name 存于 node.id（Tab 构造后设置 tab.node.id = "页名"），
	// 便于 TabBar 渲染时经类型擦除直接读取
	template<typename... Ts>
	struct TabBarData {
		std::string id;
	};
	template<typename... Ts>
	using TabBar = Control<TabBarData<Ts...>, DefaultStyle, Ts...>;

	template<typename... Ts>
	Size measureImpl(TabBarData<Ts...>&, Node& n, const Size& avail) {
		Size total{0, 0};
		for(auto* c : n.children) {
			if(!c) continue;
			Size s = c->measure(avail);
			total.w = std::max(total.w, s.w);
			total.h = std::max(total.h, s.h);
		}
		return total;
	}
	template<typename... Ts>
	void arrangeImpl(TabBarData<Ts...>&, Node& n, const Rect& r) {
		for(auto* c : n.children) {
			if(!c) continue;
			c->arrange(r);
		}
	}
	template<typename... Ts>
	void renderImpl(TabBarData<Ts...>& self, Node& n) {
		if(!ImGui::BeginTabBar(self.id.c_str())) return;
		for(auto* c : n.children) {
			if(!c) continue;
			if(ImGui::BeginTabItem(c->id.c_str())) {
				c->draw();
				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
	}

}