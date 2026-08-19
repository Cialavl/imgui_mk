// 教程 05：组合与装饰件。文档：lessons/lesson-05-combinators.md
// disabled（node.enabled）/ when（When 控件）/ 自动 ID 隔离（PushID(this)）/
// itemWidth（ItemWidth 控件）/ tooltip+colored+indent 嵌套 / EllipsisLabel。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <array>
#include <string>

int main() {
	Mk::MainWindow<> mainWindow("教程05：组合与装饰件", 1280, 800);

	bool mute = false, showAdvanced = false, isAdmin = false;
	float volume = 0.6f, dragV = 1.0f;
	int clicksA = 0, clicksB = 0;
	int frames = 0;
	std::array<char, 64> inA{}, inB{}, inC{};
	const std::string longPath =
		"C:\\Users\\someone\\Documents\\projects\\very\\deep\\dir\\file.txt";

	// —— 树组装（声明顺序：子先于父）——
	Mk::wg::SeparatorText sec1, sec2;
	sec1.self.name = "disabled vs when";
	sec2.self.name = "装饰件嵌套";

	Mk::wg::CheckBox muteBox, adminBox;
	muteBox.self.name = "静音";
	muteBox.self.checked = &mute;
	adminBox.self.name = "管理员模式";
	adminBox.self.checked = &isAdmin;
	Mk::wg::SliderF vol;
	vol.self.name = "音量（静音时禁用，但仍渲染）";
	vol.self.value = &volume;
	Mk::wg::Layout row1;
	row1.self.mode = Mk::wg::LayoutMode::H;
	row1.node.add(muteBox.node);
	row1.node.add(adminBox.node);
	row1.node.add(vol.node);

	Mk::wg::TextColored danger;
	danger.self.name = "危险操作：仅管理员可见（when 整段隐藏）";
	danger.self.color = ImVec4(0.9f, 0.4f, 0.4f, 1.0f);
	Mk::wg::When<Mk::wg::TextColored> dangerWhen;

	Mk::wg::Button btnA, btnB;
	btnA.self.setName("确认 A");
	btnA.self.onClick = [&] { ++clicksA; };
	btnB.self.setName("确认 B");
	btnB.self.onClick = [&] { ++clicksB; };
	Mk::wg::LabelText ltA, ltB;
	ltA.self.name = "A 点击";
	ltB.self.name = "B 点击";
	Mk::wg::Layout row2;
	row2.self.mode = Mk::wg::LayoutMode::H;
	row2.node.add(btnA.node);
	row2.node.add(btnB.node);

	Mk::wg::InputText<64> iwIn;
	iwIn.self.name = "定宽 220";
	iwIn.self.setName("A");
	Mk::wg::ItemWidth<Mk::wg::InputText<64>> iw;
	iw.self.width = 220.0f;

	Mk::wg::Label nested;
	nested.self.setName("悬停我可看提示：colored+indent+tooltip");
	Mk::wg::Indent indent;
	indent.self.amount = 24.0f;
	Mk::wg::Colored colored;
	colored.self.color = ImVec4(0.4f, 0.9f, 0.6f, 1.0f);
	Mk::wg::Tooltip tip;
	tip.self.name = "三层嵌套：colored+indent+tooltip";
	indent.node.add(nested.node);
	colored.node.add(indent.node);
	tip.node.add(colored.node);

	Mk::wg::EllipsisLabel path;
	path.self.name = longPath;

	Mk::wg::DragF advDrag;
	advDrag.self.name = "高级参数";
	advDrag.self.value = &dragV;
	advDrag.self.speed = 0.01f;
	Mk::wg::When<Mk::wg::DragF> advWhen;

	Mk::wg::LabelText ltFrame;
	ltFrame.self.name = "帧";

	Mk::wg::Layout root;
	root.self.mode = Mk::wg::LayoutMode::V;
	root.node.add(sec1.node);
	root.node.add(row1.node);
	root.node.add(dangerWhen.node);
	root.node.add(row2.node);
	root.node.add(ltA.node);
	root.node.add(ltB.node);
	root.node.add(iw.node);
	root.node.add(tip.node);
	root.node.add(path.node);
	root.node.add(advWhen.node);
	root.node.add(ltFrame.node);

	mainWindow.setUI([&] {
		++frames;
		// 每帧同步动态绑定
		vol.node.enabled = !mute;
		dangerWhen.self.show = isAdmin;
		advWhen.self.show = showAdvanced;
		ltA.self.value = std::to_string(clicksA);
		ltB.self.value = std::to_string(clicksB);
		ltFrame.self.value = std::to_string(frames);
		root.layout(Mk::wg::Rect{20, 20, 900, 760});
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}
