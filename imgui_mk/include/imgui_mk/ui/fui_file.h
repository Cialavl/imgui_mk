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

// 函数式控件库·文件对话框模块（Windows）：Win32 IFileOpenDialog 的异步封装。
// 对话框跑在专用 worker 线程（线程内自备 STA COM），主渲染循环不阻塞、不冻结；
// 主循环每帧调 poll() 取结果——完成后返回 true 并给出选中路径（用户取消为空串）。
//
//   Mk::fui::FileDialog dlg;
//   dlg.open("打开文件", {{"文本", "*.txt;*.md"}, {"所有文件", "*.*"}});
//   ...
//   std::string path;
//   if(dlg.poll(path) && !path.empty()) { /* 读取文件 */ }
//
// 线程与生命周期：busy 期间重复 open() 为空操作；析构会等待已打开的对话框
// 关闭（用户操作后才返回），因此对象存活期应覆盖引擎 run() 循环。

#pragma comment(lib, "ole32.lib")

namespace Mk::fui {

	class FileDialog {
	public:
		struct Filter {
			std::string name;     // 过滤器显示名，如 "文本文件"
			std::string pattern;  // 分号分隔，如 "*.txt;*.md"
		};

		FileDialog() = default;
		FileDialog(const FileDialog&) = delete;
		FileDialog& operator=(const FileDialog&) = delete;
		~FileDialog() {
			if(m_Worker.joinable()) m_Worker.join();
		}

		// 启动文件选择（busy 中调用为空操作）。filters 为空时不过滤（全部文件）
		void open(std::string title = "打开文件", std::vector<Filter> filters = {}) {
			if(m_State == State::Opening) return;
			if(m_Worker.joinable()) m_Worker.join();
			m_Path.clear();
			m_State = State::Opening;
			m_Worker = std::thread(&FileDialog::run, this,
				std::move(title), std::move(filters));
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

		void run(std::string title, std::vector<Filter> filters) {
			// worker 线程自备 STA COM（对话框需要），结束即卸载
			HRESULT hrCo = CoInitializeEx(nullptr,
				COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
			std::string result;

			IFileOpenDialog* dlg = nullptr;
			if(SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr,
				   CLSCTX_INPROC_SERVER, IID_IFileOpenDialog, (void**)&dlg))) {
				dlg->SetTitle(toWide(title).c_str());
				DWORD opts = 0;
				dlg->GetOptions(&opts);
				dlg->SetOptions(opts | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST);

				if(!filters.empty()) {
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

				if(SUCCEEDED(dlg->Show(nullptr))) {   // 无父窗口：跨线程不可传主窗口句柄
					IShellItem* item = nullptr;
					if(SUCCEEDED(dlg->GetResult(&item)) && item) {
						PWSTR p = nullptr;
						if(SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &p)) && p) {
							result = toUtf8(p);
							CoTaskMemFree(p);
						}
						item->Release();
					}
				}
				dlg->Release();
			}

			if(SUCCEEDED(hrCo)) CoUninitialize();
			m_Path = std::move(result);
			m_State.store(State::Done, std::memory_order_release);
		}
#else
		void run(std::string, std::vector<Filter>) {
			m_State.store(State::Done, std::memory_order_release);
		}
#endif

		std::thread m_Worker;
		std::atomic<State> m_State{State::Idle};
		std::string m_Path;   // worker 写入先于 state=Done（release/acquire 保序）
	};

}