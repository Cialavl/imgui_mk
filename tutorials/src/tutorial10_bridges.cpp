// 教程 10：双库互操作。文档：lessons/lesson-10-bridges.md
// 正向桥 dyn（函数式树嵌 OOP 动态列表）+ 反向桥 asWidget（OOP 容器嵌
// 函数式视图）+ 决策练习区（两区数据互换）。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/widget.h>
#include <imgui_mk/ui/fui.h>
#include <memory>
#include <string>
#include <vector>

int main() {
	Mk::MainWindow<> mainWindow("教程10：双库互操作", 1280, 800);

	int frames = 0, clicks = 0, added = 0;

	// §10.2 OOP 动态列表（长期存活；函数式按钮驱动它）
	Mk::ui::VBox listZone(0.0f);
	std::vector<std::unique_ptr<Mk::ui::Label>> pool;

	// §10.3 OOP 容器 + asWidget 反向桥（适配器长期存活）
	Mk::ui::VBox oopBox(4.0f);
	auto adapter = Mk::fui::asWidget([&] {
		return Mk::fui::hbox(8.0f,
			Mk::fui::label("函数式视图（asWidget）"),
			Mk::fui::button("点我", [&] { ++clicks; }),
			Mk::fui::labelText("计数", std::to_string(clicks)));
	});
	oopBox.add(adapter);

	// §10.4 决策练习：函数式声明区（与 OOP 列表数据互换的"镜像"）
	int mirrorCount = 0;

	mainWindow.setUI([&] {
		++frames;
		using namespace Mk::fui;

		draw(vbox(8.0f,
			separatorText("§10.2 dyn：函数式树里嵌 OOP 动态列表"),
			hbox(8.0f,
				button("添加 OOP 项", [&] {
					pool.push_back(std::make_unique<Mk::ui::Label>(
						("OOP 项 #" + std::to_string(++added)).c_str()));
					listZone.add(*pool.back());     // 渲染回调中 add（墓碑安全）
				}),
				button("删除末项", [&] {
					if(pool.empty()) return;
					listZone.remove(*pool.back());
					pool.pop_back();
				}),
				button("镜像到函数式区", [&] { mirrorCount = (int)pool.size(); })),
			subwindow("OOP 列表（动态增删）", ImVec2(0, 110), true,
				dyn(listZone)),                     // ← 正向桥

			separatorText("§10.3 asWidget：OOP 容器里嵌函数式视图"),
			subwindow("OOP 容器", ImVec2(0, 60), true,
				dyn(oopBox)),                       // oopBox 本身是 OOP 树

			separatorText("§10.4 决策练习：函数式声明区"),
			labelText("镜像项数", std::to_string(mirrorCount)),
			when(mirrorCount > 0, textColored("镜像区（when 修饰器控制显隐）",
				ImVec4(0.4f, 0.9f, 0.6f, 1.0f))),
			labelText("帧", std::to_string(frames))));
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}