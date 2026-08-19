#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <cstdio>
#include <cstddef>

int main() {
	Mk::MainWindow<> win("qt-layout smoke", 800, 600);
	int frames = 0;
	Mk::wg::VLayout box;
	Mk::wg::Label a, b;
	a.self.setName("AAAAAAAA"); b.self.setName("B");
	Mk::wg::Button btn;
	btn.self.setName("Click");
	box.addWidget(a.node);
	box.addWidget(b.node, 2.0f);
	box.addWidget(btn.node);
	Mk::wg::HLayout row;
	Mk::wg::Label c, d;
	c.self.setName("CC"); d.self.setName("DDDD");
	row.addWidget(c.node);
	row.addWidget(d.node);
	box.addWidget(row.node);
	float aH = 0, bH = 0, rowW = 0, hw = 0, hh = 0;
	std::size_t bk = 0;
	win.setUI([&] {
		++frames;
		box.layout(Mk::wg::Rect{10, 10, 400, 300});
		if(frames == 25) {
			aH = a.node.bounds.h; bH = b.node.bounds.h; rowW = row.node.bounds.w;
			hw = a.node.hint().w; hh = a.node.hint().h; bk = box.node.children.size();
		}
		if(frames >= 30) glfwSetWindowShouldClose(win.getHandle(), true);
	});
	Mk::mkui<Mk::window>::run();
	std::printf("bk=%zu hint=%.1fx%.1f aH=%.1f bH=%.1f rowW=%.1f\n", bk, hw, hh, aH, bH, rowW);
	return 0;
}
