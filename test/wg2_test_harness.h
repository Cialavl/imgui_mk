#pragma once
// wg2 单元测试的离线 ImGui 夹具：不依赖 GLFW/GL 的上下文 + 帧循环 + 输入注入。
// 每个 Harness 实例各建一个 ImGui 上下文，测试之间互不干扰。

#include <imgui.h>
#include <imgui_mk/ui/wg2/style.h>   // applyTheme / Theme / dpiScale
#include <imgui_mk/ui/wg2/wg2.h>

namespace wg2test {

struct Harness {
	Harness() {
		Mk::wg2::clearChrome();   // 引擎状态按控件地址登记：每例从干净状态开始
		Mk::wg2::clearTextures(); // 默认纹理按 ImGui 上下文登记
		Mk::wg2::clearFonts();    // 等宽字体 provider
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(800.0f, 600.0f);
		io.DeltaTime = 1.0f / 60.0f;
		io.IniFilename = nullptr;
		io.LogFilename = nullptr;
		ImFontConfig cfg;
		cfg.SizePixels = 16.0f;
		io.Fonts->AddFontDefault(&cfg);
		io.Fonts->Build();
		Mk::wg2::applyTheme(ImGui::GetCurrentContext(), Mk::wg2::Theme::dark());
	}

	~Harness() {
		if (ImGui::GetCurrentContext()) ImGui::DestroyContext();
		Mk::wg2::clearChrome();   // 栈控件的地址会复用：退出时清掉登记
	}

	Harness(const Harness&) = delete;
	Harness& operator=(const Harness&) = delete;

	ImGuiIO& io() { return ImGui::GetIO(); }
	int frameCount() const { return frameCount_; }

	// 开一帧：默认建一个铺满显示区的宿主窗口（控件必须落在窗口里才能渲染）
	void beginFrame(bool hostWindow = true) {
		ImGui::NewFrame();
		hostWindow_ = hostWindow;
		if (hostWindow_) {
			ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
			ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
			ImGui::Begin("wg2-test-host", nullptr,
				ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_NoScrollbar);
		}
		inFrame_ = true;
	}

	void endFrame() {
		if (inFrame_ && hostWindow_) ImGui::End();
		ImGui::Render();
		inFrame_ = false;
		++frameCount_;
	}

	// hint/draw 都可能用到字体度量，统一在帧内执行
	template <class F>
	void inFrame(F&& fn, bool hostWindow = true) {
		beginFrame(hostWindow);
		fn();
		endFrame();
	}

	// 一帧内把控件布局到指定矩形
	template <class W>
	void layoutFrame(W& w, const Mk::wg2::Rect& r, bool hostWindow = true) {
		inFrame([&] { Mk::wg2::layout(w, r); }, hostWindow);
	}

	// —— 输入注入（下一次 NewFrame 生效）——
	void mouseTo(float x, float y) { ImGui::GetIO().AddMousePosEvent(x, y); }
	void mouseDown(int button = 0) { ImGui::GetIO().AddMouseButtonEvent(button, true); }
	void mouseUp(int button = 0) { ImGui::GetIO().AddMouseButtonEvent(button, false); }
	void keyDown(ImGuiKey k) { ImGui::GetIO().AddKeyEvent(k, true); }
	void keyUp(ImGuiKey k) { ImGui::GetIO().AddKeyEvent(k, false); }

	// 单击：建状态 → 悬停 → 按下 → 松开（frameBody 每帧绘制同一控件树）
	template <class F>
	void click(float x, float y, F&& frameBody) {
		inFrame(frameBody);          // 建立 item 状态
		mouseTo(x, y);
		inFrame(frameBody);          // 悬停
		mouseDown();
		inFrame(frameBody);          // 按下
		mouseUp();
		inFrame(frameBody);          // 松开 → 触发
	}

	// 输入文本（下一帧生效；目标需已获得焦点）
	void typeText(const char* utf8) { ImGui::GetIO().AddInputCharactersUTF8(utf8); }

private:
	bool inFrame_ = false;
	bool hostWindow_ = true;
	int frameCount_ = 0;
};

// 记录布局分配到的矩形（布局断言用），不产生任何绘制
template <Mk::wg2::FixedString Id>
struct RectProbe {
	static constexpr auto id = Id;

	float w = 0.0f, h = 0.0f;
	Mk::wg2::Rect last{};
	int drawCount = 0;

	Mk::wg2::Size hint(const Mk::wg2::Size&) const { return Mk::wg2::Size{w, h}; }
	Mk::wg2::Policy policy() const { return Mk::wg2::Policy{}; }
	void draw(const Mk::wg2::Rect& r) { last = r; ++drawCount; }
};


// ============ 通用测试数据与探针 ============

// 下拉 / 单选 / 列表 共用的选项表（长寿命，满足控件对 const char* const* 的要求）
inline const char* const kItems[] = {"Alpha", "Beta", "Gamma"};

// 可默认构造的绘制回调（FnWidget 用例；FnWidget 要求 F 可默认构造）
struct LambdaDrawer {
	int* calls = nullptr;
	void operator()() const { if (calls) ++(*calls); }
};

// 把布局分配到的矩形写回外部变量：动态容器内部的控件拿不到引用，只能靠它断言排布
struct CaptureProbe {
	static constexpr auto id = Mk::wg2::Id("capture");
	float w = 10.0f, h = 10.0f;
	Mk::wg2::Rect* out = nullptr;

	Mk::wg2::Size hint(const Mk::wg2::Size&) const { return Mk::wg2::Size{w, h}; }
	Mk::wg2::Policy policy() const { return Mk::wg2::Policy{}; }
	void draw(const Mk::wg2::Rect& r) { if (out) *out = r; }
};

// ============ 引擎层假窗口（不创建 GLFW/GL）============

struct SharedState {
	int counter = 7;
};

struct OtherState {
	double value = 0.0;
};

// 消息循环调度：计数 render，达到 maxRenders 后关闭（0 = 立即关闭）
struct LoopImpl {
	int renderCount = 0;
	int maxRenders = 0;
	bool closed = false;
	void render() { ++renderCount; }
	bool shouldClose() const { return closed || maxRenders == 0 || renderCount >= maxRenders; }
	void close() { closed = true; }
};
struct LoopWindow { using Impl = LoopImpl; };

// 广播：记录 requestTheme / requestRescale
struct BroadcastImpl {
	int themes = 0;
	float lastScale = 0.0f;
	void requestTheme(const Mk::wg2::Theme&) { ++themes; }
	void requestRescale(float s) { lastScale = s; }
	void render() {}
	bool shouldClose() const { return true; }
	void close() {}
};
struct BroadcastWindow { using Impl = BroadcastImpl; };

// 最小契约 Impl：没有 requestTheme/requestRescale/m_Handle → 广播走编译期 no-op 分支
struct MinimalImpl {
	void render() {}
	bool shouldClose() const { return true; }
	void close() {}
};
struct MinimalWindow { using Impl = MinimalImpl; };

// UTF-8 合法性检查（ellipsize 不能在码点中间切断）
inline bool isValidUtf8(const std::string& s) {
	std::size_t i = 0;
	while (i < s.size()) {
		const unsigned char c = static_cast<unsigned char>(s[i]);
		std::size_t len = 0;
		if (c < 0x80) len = 1;
		else if ((c >> 5) == 0x06) len = 2;
		else if ((c >> 4) == 0x0E) len = 3;
		else if ((c >> 3) == 0x1E) len = 4;
		else return false;
		if (i + len > s.size()) return false;
		for (std::size_t k = 1; k < len; ++k) {
			if ((static_cast<unsigned char>(s[i + k]) >> 6) != 0x02) return false;
		}
		i += len;
	}
	return true;
}

} // namespace wg2test
