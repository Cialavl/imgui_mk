#pragma once
// p20 文件对话框：FileDialog（Win32 异步）
#include "../gallery_chrome.h"

namespace gallery {

struct PageFile {
	Button fileOpen;
	LabelText fileSize;
	EllipsisLabel filePath;
	LabelText fileBinText, fileMissText;
	When<LabelText> fileBin, fileMiss;

	Page page;
	PageFile(State& s) {
		page.setup("文件对话框",
			"FileDialog：Win32 IFileOpenDialog 异步封装（worker 线程）；"
			"帧循环每帧 poll() 取结果。对话框期间不冻结。",
			snips::kFile);
		fileOpen.self.setName("选文件…");
		fileOpen.self.onClick = [&] { s.openFileDlg(); };
		fileSize.self.name = "文件大小";
		fileSize.self.valueFn = [&s] { return std::to_string(s.fileSize) + " bytes"; };
		filePath.self.name = "(none)";
		filePath.self.valueFn = [&s] { return s.filePath.empty() ? std::string("(none)") : s.filePath; };
		fileBinText.self.name = "二进制";
		fileMissText.self.name = "错误";
		fileMissText.self.value = "文件不可读";
		fileBin.node.add(fileBinText.node);
		fileMiss.node.add(fileMissText.node);
		page.addWidget(fileOpen);
		page.addWidget(fileSize);
		page.addWidget(filePath);
		page.addWidget(fileBin);
		page.addWidget(fileMiss);
	}

	void refresh(State& s) {
		fileBin.self.show = !s.filePath.empty() && s.fileBinary;
		fileBinText.self.value = "二进制文件，预览不显示内容";
		fileMiss.self.show = s.fileMissing;
	}
};

} // namespace gallery
