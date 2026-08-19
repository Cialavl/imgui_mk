// 教程 10：桥接与动态树。文档：lessons/lesson-10-bridges.md
// 动态列表（运行时 add/remove 墓碑机制）+ FnRender（裸 ImGui 绘制函数桥接为 wg 节点）+
// 数据镜像（When 条件联动）。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <memory>
#include <string>
#include <vector>

int main() {
	Mk::MainWindow<> mainWindow("教程10：桥接与动态树", 1280, 800);

	int frames = 0, clicks = 0, added = 0;

	// —— FnRender 桥：wg 树内嵌裸 ImGui 绘制（可默认构造 callable + 指针捕获）——
	struct CounterDrawerImpl {
		int* clicks = nullptr;
		void operator()() {
			if(clicks) ImGui::Text("FnRender 内裸绘制：点击 %d 次", *clicks);
		}
	};
	Mk::wg::FnRender<CounterDrawerImpl> fnView;
	fnView.self.fn = CounterDrawerImpl{&clicks};

	// —— 镜像计数 ——
	int mirrorCount = 0;

	// —— 树组装（子先声明；pool/listZone 须在按钮 lambda 之前）——
	std::vector<std::unique_ptr<Mk::wg::Label>> pool;
	Mk::wg::SubWindow<> listZone;
	listZone.node.idname = "list10";
	listZone.self.height = 110.0f;

	Mk::wg::SeparatorText sec1, sec2, sec3;
	sec1.self.name = "动态列表：渲染回调内增删（墓碑机制）全程安全";
	sec2.self.name = "FnRender：裸 ImGui 绘制桥接为 wg 节点";
	sec3.self.name = "数据镜像（When 条件联动）";

	Mk::wg::Button bAdd, bDel, bMirror;
	bAdd.self.setName("添加 wg 项");
	bAdd.self.onClick = [&] {
		pool.push_back(std::make_unique<Mk::wg::Label>(
			("wg 项 #" + std::to_string(++added)).c_str()));
		listZone.node.add(pool.back()->node);   // 渲染回调内 add（墓碑安全）
	};
	bDel.self.setName("删除末项");
	bDel.self.onClick = [&] {
		if(pool.empty()) return;
		listZone.node.remove(pool.back()->node);   // 渲染回调内 remove（墓碑安全）
		pool.pop_back();
	};
	bMirror.self.setName("镜像到下方");
	bMirror.self.onClick = [&] { mirrorCount = (int)pool.size(); };
	Mk::wg::Layout toolRow;
	toolRow.self.mode = Mk::wg::LayoutMode::H;
	toolRow.node.add(bAdd.node);
	toolRow.node.add(bDel.node);
	toolRow.node.add(bMirror.node);


	Mk::wg::LabelText ltMirror, ltFrame;
	ltMirror.self.name = "镜像数量";
	ltFrame.self.name = "帧";
	Mk::wg::TextColored mirrorNote;
	mirrorNote.self.name = "镜像 > 0：when 条件成立才渲染";
	mirrorNote.self.color = ImVec4(0.4f, 0.9f, 0.6f, 1.0f);
	Mk::wg::When<Mk::wg::TextColored> mirrorWhen;


	Mk::wg::Layout root;
	root.self.mode = Mk::wg::LayoutMode::V;
	root.node.add(sec1.node);
	root.node.add(toolRow.node);
	root.node.add(listZone.node);
	root.node.add(sec2.node);
	root.node.add(fnView.node);
	root.node.add(sec3.node);
	root.node.add(ltMirror.node);
	root.node.add(mirrorWhen.node);
	root.node.add(ltFrame.node);

	mainWindow.setUI([&] {
		++frames;
		ltMirror.self.value = std::to_string(mirrorCount);
		mirrorWhen.self.show = mirrorCount > 0;
		ltFrame.self.value = std::to_string(frames);
		root.layout(Mk::wg::Rect{20, 20, 900, 760});
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}


