#pragma once
#include <imgui_mk/ui/wg.h>
#include <array>
#include <cstdio>
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


	// ============ 原子控件 ============
	// 每个原子 = 模板 struct（数据 + 功能函数），经 Control 组合。
	// 数据件含自持状态（字符串/值），功能函数含行为（render/measure/…）。
	// 布局：measureImpl 返回内容尺寸；arrangeImpl 记录 bounds。

	// —— Label（文本标签，持字符串副本）——
	template<std::size_t MaxLen = 128>
	struct LabelImpl {
		std::string text;

		void setText(const char* t) { text = t; }
		void setText(const std::string& t) { text = t; }
		const char* c_str() const { return text.c_str(); }

		void renderImpl(Node& n) { ImGui::TextUnformatted(text.c_str()); }
		Size measureImpl(Node& n, const Size& avail) {
			ImVec2 s = ImGui::CalcTextSize(text.c_str());
			return Size{s.x, s.y};
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
	};
	using Label = Control<LabelImpl<>, DefaultStyle>;

	// —— Button（按钮，点击回调）——
	struct ButtonImpl {
		std::string label;
		std::function<void()> onClick;

		void setLabel(const char* l) { label = l; }
		const char* c_str() const { return label.c_str(); }

		void renderImpl(Node& n) {
			if(ImGui::Button(label.c_str()) && onClick) onClick();
		}
		Size measureImpl(Node& n, const Size&) {
			ImVec2 s = ImGui::CalcTextSize(label.c_str());
			return Size{s.x + 20.0f, s.y + 12.0f};
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
	};
	using Button = Control<ButtonImpl, DefaultStyle>;

	// —— SliderF（滑块，绑定 float&——引用成员：控件经转发构造初始化，非 tuple 子）——
	struct SliderFImpl {
		std::string label;
		float* value = nullptr;   // 指针绑定（可默认构造，兼容 tuple 子控件）
		float mn = 0, mx = 1;

		Size measureImpl(Node& n, const Size& avail) {
			return Size{avail.w > 0 ? avail.w : 200.0f, 24.0f};
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) {
			if(value) ImGui::SliderFloat(label.c_str(), value, mn, mx);
		}
	};
	using SliderF = Control<SliderFImpl, DefaultStyle>;

	// —— CheckBox（复选框，绑定 bool*）——
	struct CheckBoxImpl {
		std::string label;
		bool* checked = nullptr;

		Size measureImpl(Node& n, const Size&) {
			return Size{ImGui::CalcTextSize(label.c_str()).x + 28.0f, 22.0f};
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) {
			if(checked) ImGui::Checkbox(label.c_str(), checked);
		}
	};
	using CheckBox = Control<CheckBoxImpl, DefaultStyle>;

	// —— InputText（文本输入，模板参数 N = 缓冲容量）——
	template<std::size_t N = 64>
	struct InputTextImpl {
		std::string label;
		std::array<char, N> buf{};

		void setText(const char* t) { std::snprintf(buf.data(), N, "%s", t); }
		const char* c_str() const { return buf.data(); }

		Size measureImpl(Node& n, const Size& avail) {
			return Size{avail.w > 0 ? avail.w : 220.0f, 24.0f};
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) {
			ImGui::InputText(label.c_str(), buf.data(), N);
		}
	};
	template<std::size_t N = 64>
	using InputText = Control<InputTextImpl<N>, DefaultStyle>;

	// —— Combo（下拉，items 长期存活）——
	struct ComboImpl {
		std::string label;
		int* current = nullptr;
		const char* const* items = nullptr;
		int count = 0;

		Size measureImpl(Node& n, const Size&) {
			return Size{220.0f, 24.0f};
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) {
			if(current && items) ImGui::Combo(label.c_str(), current, items, count);
		}
	};
	using Combo = Control<ComboImpl, DefaultStyle>;

	// —— ProgressBar（进度条，绑定 float* fraction）——
	struct ProgressBarImpl {
		float* fraction = nullptr;
		std::string overlay;

		Size measureImpl(Node& n, const Size& avail) {
			return Size{avail.w > 0 ? avail.w : 200.0f, 24.0f};
		}
		void arrangeImpl(Node& n, const Rect& r) { n.bounds = r; }
		void renderImpl(Node& n) {
			if(fraction) {
				ImGui::ProgressBar(*fraction, ImVec2(-FLT_MIN, 0),
					overlay.empty() ? nullptr : overlay.c_str());
			}
		}
	};
	using ProgressBar = Control<ProgressBarImpl, DefaultStyle>;

	// —— Tree（可折叠节点，展开才渲染子）——
	struct TreeImpl {
		std::string label;
		bool defaultOpen = false;

		void renderImpl(Node& n) {
			ImGuiTreeNodeFlags flags = defaultOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0;
			if(ImGui::TreeNodeEx(label.c_str(), flags)) {
				for(auto* c : n.children) {
					if(!c) continue;
					ImGui::SetCursorPos(ImVec2(c->bounds.x - n.bounds.x,
						c->bounds.y - n.bounds.y));
					c->draw();
				}
				ImGui::TreePop();
			}
		}
		Size measureImpl(Node& n, const Size& avail) {
			Size total{0, 0};
			for(auto* c : n.children) {
				if(!c) continue;
				Size s = c->measure(avail);
				total.h += s.h;
			}
			return Size{avail.w, total.h + 20.0f};
		}
		void arrangeImpl(Node& n, const Rect& r) {
			float y = r.y + 20.0f;   // 标题行下
			for(auto* c : n.children) {
				if(!c) continue;
				Size s = c->measure(Size{r.w, r.h - (y - r.y)});
				c->arrange(Rect{r.x, y, s.w, s.h});
				y += s.h;
			}
		}
	};
	using Tree = Control<TreeImpl, DefaultStyle>;

}