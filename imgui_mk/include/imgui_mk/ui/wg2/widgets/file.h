#pragma once
#ifdef _WIN32
#include <windows.h>
#include <objbase.h>
#include <shobjidl.h>
#include <shlwapi.h>
#endif
#include <atomic>
#include <cstdint>
#include <string>
#include <thread>
#include <utility>
#include <vector>

// Mk::wg2 文件对话框模块（Windows）：Win32 IFileDialog 的异步封装。
// 打开 / 保存 / 选文件夹都跑在专用 worker 线程（STA COM），主循环不冻结；
// 主循环每帧调 poll() 取结果——完成返回 true，路径为空串表示用户取消。
//
//   Mk::wg2::FileDialog dlg;
//   dlg.open("打开文件", {{"文本", "*.txt;*.md"}, {"所有文件", "*.*"}});
//   // 或 dlg.save("保存为", "untitled.txt");  dlg.pickFolder("选择曲库目录");
//   ...
//   std::string path;
//   if (dlg.poll(path) && !path.empty()) { /* 使用 path */ }

#pragma comment(lib, "ole32.lib")

namespace Mk::wg2 {

	class FileDialog {
	public:
		struct Filter {
			std::string name;     // 过滤器显示名，如 "文本文件"
			std::string pattern;  // 分号分隔，如 "*.txt;*.md"
		};

		enum class Kind { Open, Save, Folder };

		FileDialog() = default;
		FileDialog(const FileDialog&) = delete;
		FileDialog& operator=(const FileDialog&) = delete;
		~FileDialog() {
			if(m_Worker.joinable()) m_Worker.join();
		}

		// 打开文件（busy 中调用为空操作）
		void open(std::string title = "打开文件", std::vector<Filter> filters = {}) {
			start(Kind::Open, std::move(title), std::move(filters), {});
		}

		// 保存文件：defaultName 为建议文件名
		void save(std::string title = "保存文件", std::string defaultName = {},
			std::vector<Filter> filters = {}) {
			start(Kind::Save, std::move(title), std::move(filters), std::move(defaultName));
		}

		// 选择文件夹（曲库根目录等）
		void pickFolder(std::string title = "选择文件夹") {
			start(Kind::Folder, std::move(title), {}, {});
		}

		bool busy() const { return m_State == State::Opening; }

		// 完成则取走结果并回到空闲：返回 true，outPath 为选中路径（取消为空串）
		bool poll(std::string& outPath) {
			if(m_State != State::Done) return false;
			if(m_Worker.joinable()) m_Worker.join();
			m_State = State::Idle;
			outPath = std::move(m_Path);
			return true;
		}

	private:
		enum class State { Idle, Opening, Done };

		void start(Kind kind, std::string title, std::vector<Filter> filters, std::string defaultName) {
			if(m_State == State::Opening) return;
			if(m_Worker.joinable()) m_Worker.join();
			m_Path.clear();
			m_State = State::Opening;
			m_Worker = std::thread(&FileDialog::run, this, kind,
				std::move(title), std::move(filters), std::move(defaultName));
		}

#ifdef _WIN32
		static std::wstring toWide(const std::string& s) {
			if(s.empty()) return {};
			int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
			std::wstring w((size_t)n, L'\0');
			MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), w.data(), n);
			return w;
		}

		static std::string toUtf8(const wchar_t* w) {
			if(!w) return {};
			int n = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
			std::string s((size_t)(n > 0 ? n - 1 : 0), '\0');
			if(n > 1) WideCharToMultiByte(CP_UTF8, 0, w, -1, s.data(), n, nullptr, nullptr);
			return s;
		}

		static void applyFilters(IFileDialog* dlg, const std::vector<Filter>& filters) {
			if(!dlg || filters.empty()) return;
			std::vector<std::wstring> names, pats;
			names.reserve(filters.size());
			pats.reserve(filters.size());
			for(const auto& f : filters) {
				names.push_back(toWide(f.name));
				pats.push_back(toWide(f.pattern));
			}
			std::vector<COMDLG_FILTERSPEC> specs;
			specs.reserve(filters.size());
			for(size_t i = 0; i < filters.size(); ++i) {
				specs.push_back({names[i].c_str(), pats[i].c_str()});
			}
			dlg->SetFileTypes((UINT)specs.size(), specs.data());
		}

		static std::string resultPath(IFileDialog* dlg) {
			std::string result;
			IShellItem* item = nullptr;
			if(SUCCEEDED(dlg->GetResult(&item)) && item) {
				PWSTR p = nullptr;
				if(SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &p)) && p) {
					result = toUtf8(p);
					CoTaskMemFree(p);
				}
				item->Release();
			}
			return result;
		}

		void run(Kind kind, std::string title, std::vector<Filter> filters, std::string defaultName) {
			// worker 线程自备 STA COM（对话框需要），结束即卸载
			HRESULT hrCo = CoInitializeEx(nullptr,
				COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
			std::string result;

			if(kind == Kind::Save) {
				IFileSaveDialog* dlg = nullptr;
				if(SUCCEEDED(CoCreateInstance(CLSID_FileSaveDialog, nullptr,
					   CLSCTX_INPROC_SERVER, IID_IFileSaveDialog, (void**)&dlg))) {
					dlg->SetTitle(toWide(title).c_str());
					DWORD opts = 0;
					dlg->GetOptions(&opts);
					dlg->SetOptions(opts | FOS_FORCEFILESYSTEM | FOS_OVERWRITEPROMPT | FOS_PATHMUSTEXIST);
					if(!defaultName.empty()) dlg->SetFileName(toWide(defaultName).c_str());
					applyFilters(dlg, filters);
					if(SUCCEEDED(dlg->Show(nullptr))) result = resultPath(dlg);
					dlg->Release();
				}
			}
			else {
				IFileOpenDialog* dlg = nullptr;
				if(SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr,
					   CLSCTX_INPROC_SERVER, IID_IFileOpenDialog, (void**)&dlg))) {
					dlg->SetTitle(toWide(title).c_str());
					DWORD opts = 0;
					dlg->GetOptions(&opts);
					opts |= FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST;
					opts |= (kind == Kind::Folder) ? FOS_PICKFOLDERS : FOS_FILEMUSTEXIST;
					dlg->SetOptions(opts);
					applyFilters(dlg, filters);
					if(SUCCEEDED(dlg->Show(nullptr))) result = resultPath(dlg);
					dlg->Release();
				}
			}

			if(SUCCEEDED(hrCo)) CoUninitialize();
			m_Path = std::move(result);
			m_State.store(State::Done, std::memory_order_release);
		}
#else
		void run(Kind, std::string, std::vector<Filter>, std::string) {
			m_State.store(State::Done, std::memory_order_release);
		}
#endif

		std::thread m_Worker;
		std::atomic<State> m_State{State::Idle};
		std::string m_Path;   // worker 写入先于 state=Done（release/acquire 保序）
	};

} // namespace Mk::wg2
