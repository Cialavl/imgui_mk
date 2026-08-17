#pragma once
#include <imgui_mk/windowRendereren.h>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace Mk::ui {

	// 控件基类：控件、布局容器、子窗口统一树形接口。
	// ImGui 是立即模式：draw() 每帧被调用，提交本帧 UI 描述；
	// 子类实现 render()，必须自己配对引入的 BeginXxx/EndXxx，嵌套树由此天然正确。
	//
	// 渲染协议：外部只调 draw()（非虚），基类统一处理
	//   1) 可见性：不可见直接跳过整棵子树；
	//   2) 禁用：BeginDisabled/EndDisabled 包裹 render()；
	//   3) ID 作用域：PushID(this) 以对象地址隔离，同 label 控件不再冲突。
	//
	// 所有权：add() 存裸引用、不接管所有权，控件与其绑定变量的
	// 存活期必须覆盖整个引擎 run() 循环。
	class Widget {
	public:
		virtual ~Widget() = default;

		// 唯一公共渲染入口：由窗口/父控件每帧调用
		void draw() {
			if(!m_Visible) return;
			ImGui::PushID(this);
			if(m_Enabled) {
				render();
			}
			else {
				ImGui::BeginDisabled();
				render();
				ImGui::EndDisabled();
			}
			ImGui::PopID();
		}

		// —— 状态 ——

		void setVisible(bool visible) { m_Visible = visible; }
		bool isVisible() const { return m_Visible; }

		void setEnabled(bool enabled) { m_Enabled = enabled; }
		bool isEnabled() const { return m_Enabled; }

		// —— 动态树（渲染回调内调用安全）——

		// 挂子控件（容器/子窗口用）；返回子控件引用，支持链式 add(a).add(b)
		Widget& add(Widget& child) {
			m_Children.push_back(&child);
			return child;
		}

		// 摘除子控件：打墓碑（槽位置空），正在进行的渲染迭代跳过空槽，
		// 下一帧渲染开始前压实，因此渲染回调内调用安全
		bool remove(Widget& child) {
			for(auto& slot : m_Children) {
				if(slot == &child) {
					slot = nullptr;
					return true;
				}
			}
			return false;
		}

		// 摘除全部子控件（同样只打墓碑，帧首压实）
		void clearChildren() {
			for(auto& slot : m_Children) {
				slot = nullptr;
			}
		}

		// 有效子控件数（不含墓碑）
		std::size_t childCount() const {
			std::size_t n = 0;
			for(auto* c : m_Children) {
				if(c) ++n;
			}
			return n;
		}

	protected:
		// 本控件实际 UI 提交（子类必须实现；不要直接调用，走 draw()）
		virtual void render() = 0;

		// 渲染全部有效子控件（顺序布局用，无间距语义）
		void renderChildren() {
			forEachChild([](Widget& child) { child.draw(); });
		}

		// 逐个访问有效子控件，供需要自定间距/换行的容器使用：
		// 1) 访问前压实墓碑；2) 孩子数量取快照，本帧内新增的孩子下一帧再访问；
		// 3) 按索引重新取槽位，回调内 add() 触发扩容也不会失效
		template <typename Fn>
		void forEachChild(Fn&& fn) {
			compact();
			const std::size_t n = m_Children.size();
			for(std::size_t i = 0; i < n; ++i) {
				if(m_Children[i]) {
					fn(*m_Children[i]);
				}
			}
		}

	private:
		// 清除墓碑槽位（只在逐孩子访问开始前调用）
		void compact() {
			m_Children.erase(
				std::remove(m_Children.begin(), m_Children.end(), nullptr),
				m_Children.end());
		}

		bool m_Visible = true;
		bool m_Enabled = true;
		std::vector<Widget*> m_Children;
	};

	// —— 布局容器 ——

	// 垂直布局：子控件从上到下依次排列，空隙只在项间（无尾部空隙）。
	// spacing < 0 用 ImGui 默认行距；== 0 紧凑无空隙；> 0 精确像素空隙
	class VBox : public Widget {
	public:
		explicit VBox(float spacing = -1.0f) : m_Spacing(spacing) {}

		void setSpacing(float spacing) { m_Spacing = spacing; }

	protected:
		void render() override {
			bool first = true;
			forEachChild([&](Widget& child) {
				if(!first) gap();
				first = false;
				child.draw();
			});
		}

	private:
		void gap() const {
			if(m_Spacing < 0.0f) {
				ImGui::Spacing();
			}
			else if(m_Spacing > 0.0f) {
				ImGui::Dummy(ImVec2(0.0f, m_Spacing));
			}
			// == 0：紧凑，不加空隙
		}

		float m_Spacing;
	};

	// 水平布局：子控件从左到右排在同一行，空隙只在项间（无尾部 SameLine）。
	// spacing < 0 用 ImGui 默认间距；>= 0 精确像素间距
	class HBox : public Widget {
	public:
		explicit HBox(float spacing = -1.0f) : m_Spacing(spacing) {}

		void setSpacing(float spacing) { m_Spacing = spacing; }

	protected:
		void render() override {
			bool first = true;
			forEachChild([&](Widget& child) {
				if(!first) {
					ImGui::SameLine(0.0f, m_Spacing);
				}
				first = false;
				child.draw();
			});
		}

	private:
		float m_Spacing;
	};

	// 分组：把多个子控件打包成一个整体参与外部布局
	// （例如“标签+输入框”多行打成一个水平单元），不产生可见边框
	class Group : public Widget {
	protected:
		void render() override {
			ImGui::BeginGroup();
			renderChildren();
			ImGui::EndGroup();
		}
	};

	// 折叠容器：标题栏可点击展开/收起，展开时才渲染子控件。
	// flags 传 ImGuiTreeNodeFlags（如 ImGuiTreeNodeFlags_DefaultOpen）
	class CollapsibleHeader : public Widget {
	public:
		explicit CollapsibleHeader(const char* label, ImGuiTreeNodeFlags flags = 0)
			: m_Label(label), m_Flags(flags) {}

		void setLabel(const char* label) { m_Label = label; }

	protected:
		void render() override {
			if(ImGui::CollapsingHeader(m_Label.c_str(), m_Flags)) {
				renderChildren();
			}
		}

	private:
		std::string m_Label;
		ImGuiTreeNodeFlags m_Flags;
	};

	// —— 嵌套子窗口 ——

	// 子窗口：嵌在父窗口内容区内（BeginChild），内部可再挂任意控件；
	// 可独立滚动、可带边框。非 docking 分支下“窗口嵌套”的标准实现。
	// id/尺寸/边框运行时可变。
	class SubWindow : public Widget {
	public:
		SubWindow(const char* id, const ImVec2& size = ImVec2(0, 0), bool border = true)
			: m_Id(id), m_Size(size), m_Border(border) {
		}

		void setId(const char* id) { m_Id = id; }
		void setSize(const ImVec2& size) { m_Size = size; }
		void setBorder(bool border) { m_Border = border; }

	protected:
		void render() override {
			if(ImGui::BeginChild(m_Id.c_str(), m_Size, m_Border ? ImGuiChildFlags_Border : 0)) {
				renderChildren();
			}
			ImGui::EndChild();
		}

	private:
		std::string m_Id;
		ImVec2 m_Size;
		bool m_Border;
	};

	// —— 基础控件 ——

	// 文本标签（显示）。持字符串副本：setText 传临时字符串也安全
	class Label : public Widget {
	public:
		Label(const char* text) : m_Text(text) {}

		void setText(std::string text) { m_Text = std::move(text); }
		const char* text() const { return m_Text.c_str(); }

	protected:
		void render() override {
			ImGui::TextUnformatted(m_Text.c_str());
		}

	private:
		std::string m_Text;
	};

	// 按钮（动作绑定）：点击发生的那一帧 ImGui::Button 返回 true，转发回调
	class Button : public Widget {
	public:
		Button(const char* label, std::function<void()> onClick = nullptr)
			: m_Label(label), m_OnClick(std::move(onClick)) {}

		void setLabel(const char* label) { m_Label = label; }

	protected:
		void render() override {
			if(ImGui::Button(m_Label.c_str()) && m_OnClick) {
				m_OnClick();
			}
		}

	private:
		std::string m_Label;
		std::function<void()> m_OnClick;
	};

	// 滑块（数据绑定）：持有外部 float 引用，每帧读引用画滑块、拖动时 ImGui 直接写回
	class SliderF : public Widget {
	public:
		SliderF(const char* label, float& value, float mn, float mx)
			: m_Label(label), m_Value(value), m_Min(mn), m_Max(mx) {}

	protected:
		void render() override {
			ImGui::SliderFloat(m_Label.c_str(), &m_Value, m_Min, m_Max);
		}

	private:
		std::string m_Label;
		float& m_Value;   // 引用绑定：状态在用户代码里
		float m_Min, m_Max;
	};

	// 复选框（数据绑定）：绑定外部 bool 引用
	class CheckBox : public Widget {
	public:
		CheckBox(const char* label, bool& checked)
			: m_Label(label), m_Checked(checked) {}

	protected:
		void render() override {
			ImGui::Checkbox(m_Label.c_str(), &m_Checked);
		}

	private:
		std::string m_Label;
		bool& m_Checked;
	};

}