#pragma once
#include <fstream>
#include <filesystem>
// p20 文件对话框：FileDialog（Win32 异步，wg2 页面）
#include "../gallery_chrome.h"

namespace gallery {

struct PageFile {
using Root = Mk::wg2::VBox<
Mk::wg2::Id("root"),
Mk::wg2::Button<Mk::wg2::Id("open")>,
Mk::wg2::LabelText<Mk::wg2::Id("size")>,
Mk::wg2::EllipsisLabel<Mk::wg2::Id("path")>,
Mk::wg2::When<Mk::wg2::Id("binary"),
Mk::wg2::LabelText<Mk::wg2::Id("binaryText")>
>,
Mk::wg2::When<Mk::wg2::Id("missing"),
Mk::wg2::LabelText<Mk::wg2::Id("missingText")>
>
>;

Page page;
Root root;

	// 本页自己的数据（不共享、不依赖外部状态容器）
	Mk::wg2::FileDialog dlg;
	std::string filePath;
	std::uintmax_t fileSize = 0;
	bool fileBinary = false;
	bool fileMissing = false;

PageFile(Mk::window* win) {
page.setup("文件对话框",
"FileDialog：Win32 IFileOpenDialog 异步封装（worker 线程）；"
"帧循环每帧 poll() 取结果。对话框期间不冻结。",
snips::kFile);

root.at<Mk::wg2::Id("open")>().label = "选文件…";
root.at<Mk::wg2::Id("open")>().onClick = [this] { openDialog(); };
root.at<Mk::wg2::Id("size")>().name = "文件大小";
root.at<Mk::wg2::Id("size")>().valueFn = [&] { return std::to_string(fileSize) + " bytes"; };
root.at<Mk::wg2::Id("path")>().text = "(none)";
root.at<Mk::wg2::Id("path")>().valueFn = [&] { return filePath.empty() ? std::string("(none)") : filePath; };
root.at<Mk::wg2::Id("binary"), Mk::wg2::Id("binaryText")>().name = "二进制";
root.at<Mk::wg2::Id("missing"), Mk::wg2::Id("missingText")>().name = "错误";
root.at<Mk::wg2::Id("missing"), Mk::wg2::Id("missingText")>().value = "文件不可读";
	root.at<Mk::wg2::Id("binary")>().showFn = [&] { return !filePath.empty() && fileBinary; };
	root.at<Mk::wg2::Id("missing")>().showFn = [&] { return fileMissing; };

	// 对话框与结果都归本页；主菜单通过 OpenFile 事件通知本页
	auto& engine = Mk::mkui<Mk::window>::RegistrationWindow();
	engine.on<OpenFile>([this](const OpenFile&) { openDialog(); });
	win->watchFileDialog(dlg, [this](std::string path) { loadResult(std::move(path)); });

	page.addWidget([&](const Mk::wg2::Rect& r) {
 Mk::wg2::layout(root, r);
}, [&](const Mk::wg2::Size& avail) {
return root.hint(avail);
});
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
