#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/widget.h>
#include <memory>
#include <string>
#include <vector>

namespace {

// 状态栏：每帧从绑定变量重建自身文本（数据绑定 + 持字符串副本）
class StatusBar : public Mk::ui::Widget {
public:
	StatusBar(float& volume, bool& mute, int& items, int& clicks)
		: m_Volume(volume), m_Mute(mute), m_Items(items), m_Clicks(clicks) {}

protected:
	void render() override {
		m_Text = "音量=" + std::to_string(m_Volume).substr(0, 4)
			+ "  静音=" + (m_Mute ? "是" : "否")
			+ "  列表项=" + std::to_string(m_Items)
			+ "  同名按钮点击=" + std::to_string(m_Clicks);
		ImGui::TextUnformatted(m_Text.c_str());
	}

private:
	float& m_Volume;
	bool& m_Mute;
	int& m_Items;
	int& m_Clicks;
	std::string m_Text;   // 持副本：每帧重建也安全
};

// 动态列表：按钮回调在渲染期间增删子控件（墓碑机制保证回调内改树安全）。
// 自持子控件所有权：列表项随列表对象存活，不会悬垂
class DynamicList : public Mk::ui::Widget {
public:
	explicit DynamicList(int& counter) : m_Counter(counter) {}

	void addItem() {
		m_Pool.push_back(std::make_unique<Mk::ui::Label>(
			("列表项 #" + std::to_string(++m_Counter)).c_str()));
		add(*m_Pool.back());
	}

	void removeLast() {
		if(m_Pool.empty()) return;
		remove(*m_Pool.back());   // 渲染中调用：打墓碑，本帧立即生效
		m_Pool.pop_back();
	}

protected:
	void render() override {
		if(childCount() == 0) {
			ImGui::TextUnformatted("（空列表：点“添加”放入列表项）");
			return;
		}
		renderChildren();
	}

private:
	int& m_Counter;
	std::vector<std::unique_ptr<Mk::ui::Label>> m_Pool;   // 子控件所有权池
};

} // namespace

// 控件树示例：setWidget 挂载整棵树，演示——
//   布局容器：VBox/HBox（可调间距）、Group（打包整体）、CollapsibleHeader（折叠）、SubWindow（嵌套子窗口）
//   动态树：  按钮回调在渲染期间增删子控件（墓碑安全）
//   状态：    setVisible/setEnabled 整体隐藏/禁用子树，按钮运行时改文本（setLabel）
//   ID 作用域：两个同 label“点我”按钮互不冲突、独立可点
// 窗口内容随意把玩，直接关掉窗口即可退出。
int main() {
	Mk::MainWindow<> mainWindow("控件树示例", 1280, 800);

	// —— 被绑定的状态（存活期覆盖 run()，与控件树同作用域）——
	float volume = 0.6f;
	bool  mute = false;
	bool  listVisible = true;
	bool  inputsEnabled = true;
	int   counter = 0;
	int   clicks = 0;

	// —— 控件与容器（声明顺序 = 捕获依赖顺序）——

	// 动态列表：先声明，工具行按钮回调要捕获它
	DynamicList list(counter);

	// 分组：把“提示+滑块+复选框”打包成一个整体（可整体禁用）
	Mk::ui::Group settings;
	Mk::ui::Label hint("音量设置（“禁用设置”可整体禁用本组）");
	Mk::ui::SliderF vol("音量", volume, 0.0f, 1.0f);
	Mk::ui::CheckBox muteBox("静音", mute);

	// 状态栏：每帧刷新绑定变量的当前值
	StatusBar status(volume, mute, counter, clicks);

	// 工具行：“添加/删除”在渲染回调里改控件树；“隐藏/禁用”切子树状态并改自身文本
	Mk::ui::HBox toolbar(8.0f);
	Mk::ui::Button addBtn("添加", [&] { list.addItem(); });
	Mk::ui::Button delBtn("删除", [&] { list.removeLast(); });
	Mk::ui::Button visBtn("隐藏列表", [&] {
		listVisible = !listVisible;
		list.setVisible(listVisible);
		visBtn.setLabel(listVisible ? "隐藏列表" : "显示列表");
	});
	Mk::ui::Button enBtn("禁用设置", [&] {
		inputsEnabled = !inputsEnabled;
		settings.setEnabled(inputsEnabled);
		enBtn.setLabel(inputsEnabled ? "禁用设置" : "启用设置");
	});

	// 同 label 按钮对：PushID(this) 作用域隔离，两个“点我”都能独立点击
	Mk::ui::HBox sameRow(8.0f);
	Mk::ui::Button sameA("点我", [&] { ++clicks; });
	Mk::ui::Button sameB("点我", [&] { ++clicks; });

	// 折叠容器内嵌子窗口，子窗口内放动态列表（多层容器嵌套）
	Mk::ui::CollapsibleHeader advanced("高级：折叠容器 > 嵌套子窗口 > 动态列表",
		ImGuiTreeNodeFlags_DefaultOpen);
	Mk::ui::SubWindow listPanel("列表子窗口", ImVec2(0.0f, 240.0f), true);

	// —— 组装树并挂载 ——
	settings.add(hint);
	settings.add(vol);
	settings.add(muteBox);

	toolbar.add(addBtn);
	toolbar.add(delBtn);
	toolbar.add(visBtn);
	toolbar.add(enBtn);

	sameRow.add(sameA);
	sameRow.add(sameB);

	listPanel.add(list);
	advanced.add(listPanel);

	Mk::ui::VBox root(8.0f);
	root.add(status);
	root.add(toolbar);
	root.add(sameRow);
	root.add(settings);
	root.add(advanced);

	mainWindow.setWidget(root);

	Mk::mkui<Mk::window>::run();
	return 0;
}