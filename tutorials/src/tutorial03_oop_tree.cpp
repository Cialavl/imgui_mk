// 教程 03：wg 模板对象树。文档：lessons/lesson-03-oop-widget-tree.md
// 一棵可运行的控件树：动态列表增删 + 工具栏 + 可见性控制 + 嵌套子窗口 +
// 自定义 Driver 控件（render 里每帧驱动逻辑）。wg = 模板 struct 组合，零继承。
// 布局：唯一布局控件 Layout（子先声明、运行时 node.add，声明顺序 = 析构安全）。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <memory>
#include <string>
#include <vector>

namespace {

// 自定义控件：模板 struct（数据 + 功能函数）经 Control 组合，零继承。
// 每帧逻辑写在 renderImpl 里：刷新状态文本、同步可见性、驱动计数。
struct StatusDriverImpl {
	int* frames = nullptr;
	Mk::wg::LabelImpl<>* status = nullptr;
	bool* show = nullptr;
	Mk::wg::Node* settings = nullptr;
	std::vector<std::unique_ptr<Mk::wg::Label>>* pool = nullptr;

	void renderImpl(Mk::wg::Node&) {
		if(!frames || !status || !show || !settings || !pool) return;
		++(*frames);
		status->setName("第 " + std::to_string(*frames) + " 帧 | 列表 "
			+ std::to_string(pool->size()) + " 项（渲染回调内增删的墓碑机制全程安全）");
		settings->visible = *show;
	}
	Mk::wg::Size measureImpl(Mk::wg::Node&, const Mk::wg::Size& avail) {
		return Mk::wg::Size{avail.w, 20.0f};
	}
	void arrangeImpl(Mk::wg::Node& n, const Mk::wg::Rect& r) { n.bounds = r; }
};
using StatusDriver = Mk::wg::Control<StatusDriverImpl, Mk::wg::DefaultStyle>;

} // namespace

int main() {
	Mk::MainWindow<> mainWindow("教程03：wg 对象树", 1280, 800);

	// 状态全部外置（main 栈上，覆盖 run()）
	int frames = 0, added = 0;
	float volume = 0.6f;
	bool mute = false, showSettings = true;

	// —— 声明顺序契约：子控件先声明，父 Layout 后声明（析构反序安全）——
	// 动态列表池与列表子窗口须在按钮 lambda 之前声明（lambda 捕获引用）
	std::vector<std::unique_ptr<Mk::wg::Label>> pool;
	Mk::wg::SubWindow<> listZone;          // 列表子窗口（独立滚动）
	listZone.node.idname = "listwin";
	listZone.self.height = 200.0f;
	listZone.self.border = true;

	Mk::wg::Label status;
	status.self.setName("状态文本：每帧由 StatusDriver 更新");
	status.node.idname = "status";

	Mk::wg::Button addBtn, delBtn, toggleBtn;
	addBtn.self.setName("添加");
	addBtn.self.onClick = [&] {
		pool.push_back(std::make_unique<Mk::wg::Label>(
			("列表项 #" + std::to_string(++added)).c_str()));
		listZone.node.add(pool.back()->node);   // 渲染回调内 add 安全
	};
	delBtn.self.setName("删除末项");
	delBtn.self.onClick = [&] {
		if(pool.empty()) return;
		listZone.node.remove(pool.back()->node);   // 墓碑：回调内 remove 安全
		pool.pop_back();
	};
	toggleBtn.self.setName("显示/隐藏设置区");
	toggleBtn.self.onClick = [&] {
		showSettings = !showSettings;   // 只改状态，Driver 每帧同步 visible
	};

	Mk::wg::Label hint;
	hint.self.setName("设置组（Layout V 打包，随组显隐）");
	Mk::wg::SliderF vol;
	vol.self.name = "音量";
	vol.self.value = &volume;
	Mk::wg::CheckBox muteBox;
	muteBox.self.name = "静音";
	muteBox.self.checked = &mute;
	Mk::wg::Layout settings;              // 设置区（V）
	settings.node.idname = "settings";
	settings.node.add(hint.node);
	settings.node.add(vol.node);
	settings.node.add(muteBox.node);


	Mk::wg::Layout toolRow;                // 工具栏（H）
	toolRow.self.mode = Mk::wg::LayoutMode::H;
	toolRow.node.add(addBtn.node);
	toolRow.node.add(delBtn.node);
	toolRow.node.add(toggleBtn.node);


	// Driver（渲染时驱动整棵树的逻辑）
	StatusDriver driver;
	driver.self = StatusDriverImpl{&frames, &status.self, &showSettings, &settings.node, &pool};

	// 根 Layout（V）
	Mk::wg::Layout root;
	root.self.mode = Mk::wg::LayoutMode::V;
	root.node.add(driver.node);
	root.node.add(status.node);
	root.node.add(toolRow.node);
	root.node.add(settings.node);
	root.node.add(listZone.node);

	mainWindow.setUI([&] {
		root.layout(Mk::wg::Rect{20, 20, 1240, 760});   // measure→arrange→draw
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}


