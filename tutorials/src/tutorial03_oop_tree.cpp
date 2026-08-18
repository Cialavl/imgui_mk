// 教程 03：OOP 控件树。文档：lessons/lesson-03-oop-widget-tree.md
// 一棵可运行的树：动态列表（墓碑）+ 引用绑定 + 可见性开关 + 嵌套子窗口 +
// 自定义 Driver 控件（render() 里做每帧逻辑——setWidget 后的驱动正道）。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/widget.h>
#include <memory>
#include <string>
#include <vector>

namespace {

// §3.6 自定义控件：继承 Widget、实现 render()——setWidget 挂树后，
// 每帧逻辑（帧计数、状态文本刷新、可见性同步）就写在这里。
// （这也是第 12 课"扩展自己的控件"的预热：OOP 路线。）
class StatusDriver : public Mk::ui::Widget {
public:
	StatusDriver(int& frames, Mk::ui::Label& status, bool& showSettings,
		Mk::ui::Widget& settings, std::vector<std::unique_ptr<Mk::ui::Label>>& pool)
		: m_Frames(frames), m_Status(status), m_Show(showSettings),
		  m_Settings(settings), m_Pool(pool) {}

protected:
	void render() override {
		++m_Frames;
		m_Status.setText(
			"第 " + std::to_string(m_Frames) + " 帧 | 列表项 "
			+ std::to_string(m_Pool.size()) + " 个（渲染回调内增删，墓碑保安全）");
		m_Settings.setVisible(m_Show);   // §3.1 子树整体隐藏：一行搞定
	}

private:
	int& m_Frames;
	Mk::ui::Label& m_Status;
	bool& m_Show;
	Mk::ui::Widget& m_Settings;
	std::vector<std::unique_ptr<Mk::ui::Label>>& m_Pool;
};

} // namespace

int main() {
	Mk::MainWindow<> mainWindow("教程03：OOP 控件树", 1280, 800);

	// —— §3.3 被绑定的状态（main 栈上，活过 run()）——
	int frames = 0, added = 0;
	float volume = 0.6f;
	bool mute = false, showSettings = true;

	// —— §3.2 组装树（声明顺序 = 组装顺序，全部长期存活）——
	Mk::ui::VBox root(8.0f);
	Mk::ui::Label status("状态文本（每帧由 StatusDriver 更新）");

	Mk::ui::VBox listZone(0.0f);                              // 动态列表区
	std::vector<std::unique_ptr<Mk::ui::Label>> pool;         // 所有权池（§3.4）

	Mk::ui::HBox toolRow(8.0f);
	Mk::ui::Button addBtn("添加", [&] {
		pool.push_back(std::make_unique<Mk::ui::Label>(
			("列表项 #" + std::to_string(++added)).c_str()));
		listZone.add(*pool.back());    // §3.4 渲染回调中 add：墓碑+快照，安全
	});
	Mk::ui::Button delBtn("删除末项", [&] {
		if(pool.empty()) return;
		listZone.remove(*pool.back()); // 渲染回调中 remove：打墓碑，安全
		pool.pop_back();
	});
	Mk::ui::Button toggleBtn("显示/隐藏设置组", [&] {
		showSettings = !showSettings;  // 只改状态；树尾 Driver 每帧同步到 setVisible
	});

	Mk::ui::Group settings;
	Mk::ui::Label hint("设置组（Group 打包，整体隐藏/禁用）");
	Mk::ui::SliderF vol("音量", volume, 0.0f, 1.0f);   // §3.3 引用绑定 float
	Mk::ui::CheckBox muteBox("静音", mute);            // 引用绑定 bool

	Mk::ui::SubWindow panel("列表子窗口（独立滚动）", ImVec2(0, 0), true);
	StatusDriver driver(frames, status, showSettings, settings, pool);

	settings.add(hint);
	settings.add(vol);
	settings.add(muteBox);
	toolRow.add(addBtn);
	toolRow.add(delBtn);
	toolRow.add(toggleBtn);
	panel.add(listZone);
	root.add(status);
	root.add(toolRow);
	root.add(settings);
	root.add(panel);
	root.add(driver);   // 每帧逻辑放树尾：本帧渲染完，下一帧状态生效

	// §3.6 挂载：一行把整棵树接到窗口（setUI 的语法糖）。
	// 注意 setWidget/setUI 共用同一通道，后调用者覆盖——驱动逻辑进树（Driver）
	mainWindow.setWidget(root);

	Mk::mkui<Mk::window>::run();
	return 0;
}