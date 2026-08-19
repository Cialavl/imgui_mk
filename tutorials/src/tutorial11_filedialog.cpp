// 教程 11：异步文件对话框。文档：lessons/lesson-11-file-dialog.md
// 打开文件 → 读前 64KB → 文本/二进制判定 → 预览；对话框运行期间主循环不冻结。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/wg.h>
#include <imgui_mk/ui/wg_controls.h>
#include <imgui_mk/ui/wg_file.h>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

int main() {
	Mk::MainWindow<> mainWindow("教程11：文件对话框", 1280, 800);

	int frames = 0;
	float anim = 0.0f;
	Mk::wg::FileDialog dlg;              // 长期存活（析构等待对话框关闭）
	std::string path, preview;
	std::uintmax_t size = 0;
	bool binary = false, missing = false;

	// —— 树组装（子先声明）——
	Mk::wg::SeparatorText sec1, sec2;
	sec1.self.name = "异步对话框（worker 线程 + 主循环轮询）";
	sec2.self.name = "文本预览（前 64KB）";

	Mk::wg::Button bOpen;
	bOpen.self.setName("打开文件…");
	bOpen.self.onClick = [&] {
		dlg.open("打开文件（教程11）", {
			{"文本/代码", "*.txt;*.md;*.cpp;*.h;*.json;*.ini;*.log"},
			{"所有文件", "*.*"},
		});
	};
	Mk::wg::TextColored busyNote;
	busyNote.self.name = "对话框打开中…主循环仍在渲染";
	busyNote.self.color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
	Mk::wg::When<Mk::wg::TextColored> busyWhen;
	Mk::wg::Layout openRow;
	openRow.self.mode = Mk::wg::LayoutMode::H;
	openRow.node.add(bOpen.node);
	openRow.node.add(busyWhen.node);

	Mk::wg::LabelText ltPath, ltSize, ltFrame;
	ltPath.self.name = "路径";
	ltSize.self.name = "大小";
	ltFrame.self.name = "帧";
	Mk::wg::TextColored missNote, binNote;
	missNote.self.name = "文件不可读";
	missNote.self.color = ImVec4(0.9f, 0.4f, 0.4f, 1.0f);
	binNote.self.name = "二进制文件：仅显示大小";
	binNote.self.color = ImVec4(0.6f, 0.8f, 1.0f, 1.0f);
	Mk::wg::When<Mk::wg::TextColored> missWhen, binWhen;
	Mk::wg::Label previewText;
	Mk::wg::SubWindow<> previewWin;
	previewWin.node.idname = "prev11";
	previewWin.self.height = 240.0f;
	previewWin.node.add(previewText.node);
	Mk::wg::When<Mk::wg::SubWindow<Mk::wg::Label>> previewWhen;

	Mk::wg::Layout root;
	root.self.mode = Mk::wg::LayoutMode::V;
	root.node.add(sec1.node);
	root.node.add(openRow.node);
	root.node.add(ltPath.node);
	root.node.add(ltSize.node);
	root.node.add(missWhen.node);
	root.node.add(binWhen.node);
	root.node.add(sec2.node);
	root.node.add(previewWhen.node);
	root.node.add(ltFrame.node);

	mainWindow.setUI([&] {
		++frames;

		// 对话框期间动画继续 → 证明主循环未冻结
		anim = 0.5f + 0.5f * std::sin(frames * 0.05f);
		ImGui::ProgressBar(anim, ImVec2(-FLT_MIN, 0), "对话框打开时此条仍在动");

		// 每帧轮询结果
		std::string picked;
		if(dlg.poll(picked)) {
			path = picked;
			preview.clear();
			size = 0;
			binary = missing = false;
			if(!path.empty()) {
				std::error_code ec;
				size = std::filesystem::file_size(path, ec);
				if(ec) {
					missing = true;
				}
				else {
					std::ifstream in(path, std::ios::binary);
					std::string buf(64 * 1024, '\0');
					in.read(buf.data(), (std::streamsize)buf.size());
					buf.resize((size_t)in.gcount());
					binary = buf.find('\0') != std::string::npos;
					if(!binary) preview = std::move(buf);
				}
			}
		}

		// 每帧同步绑定
		busyWhen.self.show = dlg.busy();
		ltPath.self.value = path.empty() ? "(未选择)" : path;
		ltSize.self.value = std::to_string(size) + " 字节";
		missWhen.self.show = missing;
		binWhen.self.show = !path.empty() && binary;
		previewWhen.self.show = !path.empty() && !binary;
		previewText.self.setName(preview);
		ltFrame.self.value = std::to_string(frames);

		root.layout(Mk::wg::Rect{20, 20, 900, 760});
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}
