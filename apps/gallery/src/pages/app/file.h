#pragma once
#include <fstream>
#include <filesystem>
// p20 文件对话框：FileDialog（Win32 异步，wg2 页面）
#include "../../gallery_chrome.h"

namespace gallery {

namespace ui = Mk::wg2;   // 本文件用短名


struct PageFile {
	// 页面元数据（Page 用它填外壳）
	static constexpr const char* title = "文件对话框";
	static constexpr const char* desc  = "FileDialog：Win32 IFileOpenDialog 异步封装（worker 线程）；"
"帧循环每帧 poll() 取结果。对话框期间不冻结。";
	static constexpr std::string_view code = snips::kFile;

using Root = ui::VBox<
ui::Id("root"),
ui::Button<ui::Id("open")>,
ui::LabelText<ui::Id("size")>,
ui::EllipsisLabel<ui::Id("path")>,
ui::LabelText<ui::Id("binaryText")>,
		ui::LabelText<ui::Id("missingText")>
>;

	Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	FileDialog dlg;
	std::string filePath;
	std::uintmax_t fileSize = 0;
	bool fileBinary = false;
	bool fileMissing = false;

PageFile(Mk::window* win = nullptr) {


auto& w_open = root.at<ui::Id("open")>();
w_open.label = "选文件…";
w_open.onClick = [this] { openDialog(); };
auto& w_size = root.at<ui::Id("size")>();
w_size.name = "文件大小";
w_size.valueFn = [&] { return std::to_string(fileSize) + " bytes"; };
auto& w_path = root.at<ui::Id("path")>();
w_path.text = "(none)";
w_path.valueFn = [&] { return filePath.empty() ? std::string("(none)") : filePath; };
root.at<ui::Id("binaryText")>().name = "二进制";
auto& w_missingText = root.at<ui::Id("missingText")>();
w_missingText.name = "错误";
w_missingText.value = "文件不可读";
	root.at<ui::Id("binaryText")>().state.visibleFn = [&] { return !filePath.empty() && fileBinary; };
	root.at<ui::Id("missingText")>().state.visibleFn = [&] { return fileMissing; };

	// 对话框与结果都归本页；主菜单通过 OpenFile 事件通知本页
	auto& engine = Mk::mkui<Mk::window>::RegistrationWindow();
	engine.on<OpenFile>([this](const OpenFile&) { openDialog(); });
	if (win) win->watchFileDialog(dlg, [this](std::string path) { loadResult(std::move(path)); });

}


	void openDialog() {
		dlg.open("Open File", {
			{"Text/Code", "*.txt;*.md;*.cpp;*.h;*.hpp;*.json;*.xml;*.ini;*.log"},
			{"All files", "*.*"},
		});
	}

	void loadResult(std::string path) {
		filePath = std::move(path);
		fileBinary = false;
		fileMissing = false;
		fileSize = 0;
		if (filePath.empty()) return;
		std::error_code ec;
		fileSize = std::filesystem::file_size(filePath, ec);
		std::ifstream in(filePath, std::ios::binary);
		if (!in) { fileMissing = true; return; }
		std::string head(1024, '\0');
		in.read(head.data(), static_cast<std::streamsize>(head.size()));
		fileBinary = head.find('\0') != std::string::npos;
	}
};

} // namespace gallery
