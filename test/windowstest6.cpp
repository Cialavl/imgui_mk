#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/widget.h>
#include <memory>
#include <string>
#include <vector>

namespace {

// 状态栏：每帧从绑定变量重建文本（持字符串副本，跨窗口共享的变量实时刷新）
class StatusBar : public Mk::ui::Widget {
public:
	StatusBar(float& v, bool& m, int& logs) : m_V(v), m_M(m), m_Logs(logs) {}

protected:
	void render() override {
		m_Text = "音量=" + std::to_string(m_V).substr(0, 4)
			+ "  静音=" + (m_M ? "是" : "否")
			+ "  日志数=" + std::to_string(m_Logs);
		ImGui::TextUnformatted(m_Text.c_str());
	}

private:
	float& m_V;
	bool& m_M;
	int& m_Logs;
	std::string m_Text;
};

// 日志池：按钮回调在渲染期间向三级子窗口内的 VBox 动态追加/清空标签。
// 自持子控件所有权（unique_ptr 池），上限 12 条防无限增长
struct LogPool {
	Mk::ui::VBox& box;
	std::vector<std::unique_ptr<Mk::ui::Label>> items;
	int count = 0;

	void add() {
		if(items.size() >= 12) return;
		items.push_back(std::make_unique<Mk::ui::Label>(
			("日志 #" + std::to_string(++count) + "：嵌套子窗口内动态追加").c_str()));
		box.add(*items.back());   // 渲染回调内 add：本帧提交、下一帧渲染
	}

	void clear() {
		box.clearChildren();
		items.clear();
	}
};

} // namespace

// 嵌套多窗口示例：
//   顶层 OS 窗口 x2（引擎同时驱动，各自独立 ImGui 上下文，全关才退出）
//     工作台：一级子窗口（左/右两列）
//       左列 → 二级子窗口“调色板” → 三级子窗口“数值”（三层嵌套，控件分布各级）
//       右列 → 二级折叠容器     → 三级子窗口“日志”（渲染回调内动态增删）
//     镜像窗：二级子窗口，滑块与工作台绑定同一变量——跨窗口实时联动
int main() {
	// —— 两个顶层窗口（构造即注册进引擎）——
	Mk::MainWindow<> workbench("工作台：多层嵌套子窗口", 1400, 860);
	Mk::window mirror("第二顶层窗口：数据联动", 560, 420);

	// —— 共享绑定状态（存活期覆盖 run()）——
	float volume = 0.6f;
	bool  mute = false;

	// ============ 窗口一：工作台 ============

	// 动态日志池（先于捕获它的按钮声明）
	Mk::ui::VBox logBox(0.0f);
	LogPool logs{logBox};

	Mk::ui::VBox root(8.0f);
	StatusBar status(volume, mute, logs.count);
	Mk::ui::HBox columns(8.0f);

	// 左列：一级 → 二级 → 三层嵌套，每层都有控件
	Mk::ui::SubWindow colL("一级：左面板", ImVec2(420, 0), true);
	Mk::ui::Label l1("一级面板的控件：");
	Mk::ui::SliderF volA("音量", volume, 0.0f, 1.0f);
	Mk::ui::CheckBox muteA("静音", mute);

	Mk::ui::SubWindow sub2("二级：调色板", ImVec2(0, 240), true);
	Mk::ui::Label l2("二级子窗口内再嵌一级：");
	Mk::ui::SubWindow sub3("三级：数值", ImVec2(0, 140), true);
	Mk::ui::SliderF volB("三级音量（各层同源）", volume, 0.0f, 1.0f);
	Mk::ui::CheckBox muteB("三级静音", mute);

	// 右列：一级 → 折叠容器 → 三级日志子窗口（动态增删）
	Mk::ui::SubWindow colR("一级：右面板", ImVec2(0, 0), true);
	Mk::ui::CollapsibleHeader fold("二级：折叠容器（默认展开）", ImGuiTreeNodeFlags_DefaultOpen);
	Mk::ui::SubWindow logwin("三级：日志子窗口（独立滚动）", ImVec2(0, 260), true);
	Mk::ui::HBox logBtns(8.0f);
	Mk::ui::Button addBtn("追加日志", [&] { logs.add(); });
	Mk::ui::Button clrBtn("清空", [&] { logs.clear(); });

	// 组装左列：colL{ l1, volA, muteA, sub2{ l2, sub3{ volB, muteB } } }
	sub3.add(volB);
	sub3.add(muteB);
	sub2.add(l2);
	sub2.add(sub3);
	colL.add(l1);
	colL.add(volA);
	colL.add(muteA);
	colL.add(sub2);

	// 组装右列：colR{ fold{ logBtns, logwin{ logBox } } }
	logBtns.add(addBtn);
	logBtns.add(clrBtn);
	logwin.add(logBox);
	fold.add(logBtns);
	fold.add(logwin);
	colR.add(fold);

	columns.add(colL);
	columns.add(colR);
	root.add(status);
	root.add(columns);
	workbench.setWidget(root);

	// ============ 窗口二：镜像（跨窗口联动） ============

	Mk::ui::VBox root2(8.0f);
	Mk::ui::Label m1("本窗口与工作台绑定同一变量：拖任意滑块，两窗口全部实时联动");
	Mk::ui::SliderF volM("镜像音量", volume, 0.0f, 1.0f);
	Mk::ui::CheckBox muteM("镜像静音", mute);
	Mk::ui::SubWindow subM("二级：镜像面板", ImVec2(0, 180), true);
	Mk::ui::SliderF volM2("镜像面板内的音量（同一变量）", volume, 0.0f, 1.0f);

	subM.add(volM2);
	root2.add(m1);
	root2.add(volM);
	root2.add(muteM);
	root2.add(subM);
	mirror.setWidget(root2);

	// 引擎同时驱动两个顶层窗口；两个窗口都关闭后 run() 才返回
	Mk::mkui<Mk::window>::run();
	return 0;
}