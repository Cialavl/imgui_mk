#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/widget.h>
#include <imgui_mk/ui/fui.h>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace {

// —— 自定义函数式控件（扩展性验证）：本命名空间的 draw 重载经 ADL 被容器/修饰器
//    递归找到，不需要继承任何基类 ——

// 渲染探针：统计自己被渲染的帧数
struct RenderProbe {
	int& renders;
};
inline void draw(const RenderProbe& p) { ++p.renders; }

// 自定义计数按钮：与 withId 配合，两个同 label 实例可独立存在
struct ClickCounter {
	const char* label;
	int& clicks;
};
inline void draw(const ClickCounter& w) {
	if(ImGui::Button(w.label)) ++w.clicks;
}

// —— OOP 动态列表驱动器（dyn 桥接验证）：render() 内按固定节奏增删子控件 ——

class DynListDriver : public Mk::ui::Widget {
public:
	DynListDriver(int& frames, Mk::ui::VBox& zone,
		std::vector<std::unique_ptr<Mk::ui::Label>>& pool,
		int& adds, int& removes)
		: m_Frames(frames), m_Zone(zone), m_Pool(pool), m_Adds(adds), m_Removes(removes) {}

protected:
	void render() override {
		if(m_Frames % 10 == 5) {
			m_Pool.push_back(std::make_unique<Mk::ui::Label>(
				("动态#" + std::to_string(++m_Adds)).c_str()));
			m_Zone.add(*m_Pool.back());
		}
		if(m_Frames >= 60 && m_Frames % 10 == 0 && !m_Pool.empty()) {
			if(m_Zone.remove(*m_Pool.back())) {
				m_Pool.pop_back();
				++m_Removes;
			}
		}
	}

private:
	int& m_Frames;
	Mk::ui::VBox& m_Zone;
	std::vector<std::unique_ptr<Mk::ui::Label>>& m_Pool;
	int& m_Adds;
	int& m_Removes;
};

} // namespace

// 函数式控件树验证：setUI 回调内每帧重建整棵视图值——
//   容器：    vbox / hbox / group / collapsing / subwindow（tuple + fold 展开）
//   修饰器：  disabled（仍渲染）/ when（条件跳过）/ withId（显式 ID 作用域）
//   扩展性：  本文件自定义 RenderProbe / ClickCounter（ADL draw，无继承）
//   双向桥：  dyn() 把 OOP 动态树（墓碑增删）嵌进函数式树；asWidget() 反向嵌函数式视图
// 120 帧后自动关窗，确定性断言全过输出 PASS，退出码 0。
int main() {
	Mk::MainWindow<> mainWindow("函数式控件树验证", 1280, 800);

	// —— 外置状态（函数式核心：视图每帧重建，状态全在这里）——
	float volume = 0.8f;
	bool  mute = false;
	int   frames = 0, clicks = 0;
	int   probeAll = 0, probeCond = 0, probeDisabled = 0, probeAdapter = 0;
	int   adds = 0, removes = 0;

	// —— OOP 侧对象（dyn 桥接用，存活期覆盖 run()）——
	Mk::ui::VBox dynZone(0.0f);
	std::vector<std::unique_ptr<Mk::ui::Label>> pool;
	DynListDriver driver(frames, dynZone, pool, adds, removes);

	// 反向桥适配器：OOP 树位置，内部每帧新鲜构造函数式视图
	auto adapter = Mk::fui::asWidget([&] {
		return Mk::fui::hbox(8.0f,
			Mk::fui::label("OOP 树内嵌的函数式视图"),
			RenderProbe{probeAdapter});
	});
	Mk::ui::VBox oopZone(4.0f);
	oopZone.add(adapter);

	mainWindow.setUI([&] {
		++frames;
		volume = 0.5f + 0.5f * std::sin(frames * 0.05f);

		// 每帧构造整棵函数式视图树（值语义，帧末整体销毁）
		Mk::fui::draw(Mk::fui::vbox(8.0f,
			Mk::fui::label("帧=" + std::to_string(frames)
				+ "  音量=" + std::to_string(volume).substr(0, 4)
				+ "  动态子控件=" + std::to_string(dynZone.childCount())),
			Mk::fui::hbox(8.0f,
				Mk::fui::withId("a", ClickCounter{"同按钮", clicks}),
				Mk::fui::withId("b", ClickCounter{"同按钮", clicks})),
			Mk::fui::group(
				Mk::fui::slider("音量", volume, 0.0f, 1.0f),
				Mk::fui::checkbox("静音", mute)),
			Mk::fui::disabled(RenderProbe{probeDisabled}),       // 禁用是视觉的：仍每帧渲染
			Mk::fui::when(frames <= 60, RenderProbe{probeCond}), // 条件渲染：仅前 60 帧
			RenderProbe{probeAll},
			Mk::fui::collapsing("折叠：函数式树内嵌 OOP 动态子树", ImGuiTreeNodeFlags_DefaultOpen,
				Mk::fui::subwindow("dynbridge", ImVec2(0.0f, 180.0f), true,
					Mk::fui::dyn(dynZone))),
			Mk::fui::dyn(oopZone),   // OOP 容器（内含函数式适配器）
			Mk::fui::dyn(driver)     // 驱动器放最后：增删节奏按当帧帧号
		));

		if(frames >= 120) {
			glfwSetWindowShouldClose(mainWindow.getHandle(), true);
		}
	});

	Mk::mkui<Mk::window>::run();

	// —— 确定性断言 ——
	const bool okFrames = (frames == 120);
	const bool okAll = (probeAll == 120);
	const bool okCond = (probeCond == 60);
	const bool okDisabled = (probeDisabled == 120);
	const bool okAdapter = (probeAdapter == 120);
	const bool okDyn = (adds == 12 && removes == 7 && dynZone.childCount() == 5);
	const bool okExit = (mainWindow.getHandle() == nullptr);

	std::cout << "frames=" << frames
		<< " all=" << probeAll
		<< " cond=" << probeCond
		<< " disabled=" << probeDisabled
		<< " adapter=" << probeAdapter
		<< " adds=" << adds
		<< " removes=" << removes
		<< " dynChilds=" << dynZone.childCount() << std::endl;

	const bool pass = okFrames && okAll && okCond && okDisabled && okAdapter && okDyn && okExit;
	std::cout << (pass ? "PASS" : "FAIL")
		<< ": 帧数 " << okFrames
		<< ", 全程探针 " << okAll
		<< ", 条件渲染 " << okCond
		<< ", 禁用仍渲染 " << okDisabled
		<< ", 反向桥 " << okAdapter
		<< ", 动态桥 " << okDyn
		<< ", 干净关闭 " << okExit << std::endl;
	return pass ? 0 : 1;
}