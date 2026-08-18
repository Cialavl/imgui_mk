// 教程 11：本地文件对话框（异步状态机）。文档：lessons/lesson-11-file-dialog.md
// 打开文件 → 读前 64KB → 文本/二进制判定 → 预览。对话框期间主循环动画照跑。
#include <imgui_mk/window.h>
#include <imgui_mk/mkui.h>
#include <Mainwindow.h>
#include <imgui_mk/ui/fui.h>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>

namespace {
// §11.4 文本预览控件（ADL，第 12 课将讲三步法）
struct PreviewText { const std::string& text; };
void draw(const PreviewText& p) {
	ImGui::TextUnformatted(p.text.data(), p.text.data() + p.text.size());
}
} // namespace

int main() {
	Mk::MainWindow<> mainWindow("教程11：文件对话框", 1280, 800);

	int frames = 0;
	float anim = 0.0f;   // progressBar 绑定 float&——不能绑右值（sin 表达式），先存变量
	Mk::fui::FileDialog dlg;              // §11.5 长期存活
	std::string path, preview;
	std::uintmax_t size = 0;
	bool binary = false, missing = false;

	mainWindow.setUI([&] {
		++frames;
		using namespace Mk::fui;

		// §11.4 对话框期间动画照跑（证明主循环未冻结）
		anim = 0.5f + 0.5f * std::sin(frames * 0.05f);   // 逻辑侧更新动画值
		draw(progressBar(anim,
			ImVec2(-FLT_MIN, 0), "对话框打开时这个动画仍在动"));

		// §11.2 每帧轮询：完成则取走并装载
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

		draw(vbox(8.0f,
			separatorText("§11.2 发起异步对话框"),
			hbox(8.0f,
				button("打开文件…", [&] {
					dlg.open("打开文件（教程11）", {
						{"文本/代码", "*.txt;*.md;*.cpp;*.h;*.json;*.ini;*.log"},
						{"所有文件", "*.*"},
					});
				}),
				when(dlg.busy(), textColored("对话框打开中…（主窗口仍在渲染）",
					ImVec4(1.0f, 0.8f, 0.2f, 1.0f)))),
			labelText("路径", path.empty() ? "(未选择)" : path.c_str()),
			labelText("大小", std::to_string(size) + " 字节"),
			when(missing, textColored("文件不可读", ImVec4(0.9f, 0.4f, 0.4f, 1.0f))),
			when(!path.empty() && binary,
				textColored("二进制文件：仅显示大小", ImVec4(0.6f, 0.8f, 1.0f, 1.0f))),
			separatorText("§11.4 文本预览（前 64KB）"),
			when(!path.empty() && !binary,
				subwindow("preview", ImVec2(0, 0), true,
					PreviewText{preview})),
			labelText("帧", std::to_string(frames))));
	});

	Mk::mkui<Mk::window>::run();
	return 0;
}