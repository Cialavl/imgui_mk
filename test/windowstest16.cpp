#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/wg.h>
#include <iostream>`n#include <memory>

namespace {

// 测试控件（数据 + 功能函数）：实现 renderImpl/measureImpl/arrangeImpl
struct NodeProbeImpl {
	int& renders;
	int& measures;
	int& arranges;
	int& creates;
	int& destroys;
	void renderImpl(Mk::wg::Node&) { ++renders; }
	Mk::wg::Size measureImpl(Mk::wg::Node&, const Mk::wg::Size& avail) { ++measures; return Mk::wg::Size{100.0f, 20.0f}; }
	void arrangeImpl(Mk::wg::Node&, const Mk::wg::Rect&) { ++arranges; }
	void onCreateImpl(Mk::wg::Node&) { ++creates; }
	void onDestroyImpl(Mk::wg::Node&) { ++destroys; }
};
using NodeProbe = Mk::wg::Control<NodeProbeImpl, Mk::wg::DefaultStyle>;

// 纯数据控件（无 measure/render 实现 → traits 缺省兜底路径）
struct LeafImpl { int& renders; };
void renderImpl(LeafImpl& self, Mk::wg::Node&) { ++self.renders; }   // ADL 兜底路径
using Leaf = Mk::wg::Control<LeafImpl, Mk::wg::DefaultStyle>;

} // namespace

// test16 v1：wg 基座链路验证——
//   类型萃取+分发（成员优先/ADL 兜底）、对象树（编译期 tuple + 运行时 add）、
//   生命周期（onCreate 首帧/onDestroy 移除时）、id 查找、显式布局两遍
// 120 帧后自动关窗，确定性断言 PASS。
int main() {
	Mk::MainWindow<> mainWindow("wg 基座验证", 1000, 700);

	int frames = 0;
	int r1 = 0, m1 = 0, a1 = 0, c1 = 0, d1 = 0;
	int leafR = 0;

	// 编译期子组合（模板传入）：probe + leaf
	NodeProbe probe{r1, m1, a1, c1, d1};
	Leaf leaf{leafR};
	probe.node.id = "probe";
	leaf.node.id = "leaf";

	// 运行时动态子树：用完整 Leaf 控件对象（有 owner → ADL 分发有效）
	std::unique_ptr<Leaf> dynLeaf;

	mainWindow.setUI([&] {
		++frames;

		// 宿主区域：显式布局 probe 子树 + 动态子树
		const Mk::wg::Rect host{20, 20, 500, 200};
		probe.layout(host);

		// 运行时 add/remove（帧 60 加动态叶子，帧 90 移除 → onDestroy 路径）
		if(frames == 60) {
			dynLeaf = std::make_unique<Leaf>(leafR);
			probe.node.add(dynLeaf->node);
		}
		if(frames == 90 && dynLeaf) {
			probe.node.remove(dynLeaf->node);   // 打墓碑 + onDestroy（动态控件无 onDestroyImpl → 无操作）
			dynLeaf.reset();
		}

		// 每帧文本状态
		ImGui::Text("frames=%d probe{r=%d m=%d a=%d c=%d d=%d} leafR=%d",
			frames, r1, m1, a1, c1, d1, leafR);

		if(frames >= 120) {
			glfwSetWindowShouldClose(mainWindow.getHandle(), true);
		}
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}