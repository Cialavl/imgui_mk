# AGENTS.md

C++23 GUI library wrapping ImGui + GLFW + OpenGL. Windows-focused. `imgui/` and `glad/` are **vendored in-tree, not git submodules** (no `.gitmodules`).

## Widget system: `Mk::wg` is the ONLY UI system

Everything under `imgui_mk/include/imgui_mk/ui/` is the wg template widget system (`Mk::wg`, zero-inheritance, template-struct controls). Four headers only — do not reintroduce widget.h/fui:

- `wg.h` — 基座：`Size/Rect`, `ControlStyle/DefaultStyle`, `Node` (type-erased behavior table + tombstone children + `role`/`fill` layout marks), `ControlTraits<T>` (member-fn → ADL → fallback dispatch), `Control<Self, Style, Ts...>` composition shell (tuple children, `child<I>()`, non-copyable). `Node::layout(host)` = measure → arrange → draw; `Node::fill` keeps host size (splits); `Node::role` (CellRole) marks Fixed/Stretch split cells.
- `wg_controls.h` — **Qt 式布局体系**：独立布局类型 `VLayout/HLayout/GridLayout/WrapLayout/SplitHLayout/SplitVLayout/OverlayLayout`（共享盒式求解器 detail::collectItems/solveRow；`addWidget(w[,stretch])`/`addStretch(w)`/`setSpacing`/`setContentsMargins`/`setCols`；布局恒 fill 宿主；**缺省哨兵**：spacing/marginX/marginY 默认 -1 = 回落 ImGui 样式 ItemSpacing/WindowPadding（主题/字号联动），显式值覆盖、-1 还原——纯逻辑测试须显式定值（wg_test/test20 用 setSpacing(8)+setContentsMargins(0,0)））+ **尺寸协议**：控件实现 `sizeHintImpl/minimumSizeHintImpl/policyImpl`（Policy{Fixed/Minimum/Maximum/Preferred/Expanding/Ignored}×两方向）与 HFW（heightForWidthImpl，换行类）；`setFixedWidth/Height/Size`（min=max=值）、`setPrefSize`（hint 覆盖）、`setMinSize/setMaxSize`（**尺寸覆盖均为设计像素，collectItems 统一 em 缩放**：随字号/DPI 等比、字号 18 恒等）；旧 `measureImpl(avail)` 协议保留为兼容桥（无 sizeHintImpl 时 fallback）。原子/容器/装饰件/工具清单同前（em()/em2()/ellipsize/loadTextureRGBA/deleteTexture/FnRender<F>）。
- `wg_style.h` — `Mk::wg::Theme` (dark/light/withX chain), `applyTheme/defaultTheme/setDefaultTheme/dpiScale/scaledForMonitor`, `detail::pushTheme` (Styled uses it, pop pairs 26 colors + 3 vars).
- `wg_file.h` — `Mk::wg::FileDialog` (Win32 async IFileOpenDialog, worker thread, `open/busy/poll`).
- `wg_extras.h` — 可选扩展层（应用层骨架，核心四头保持精简）: `Sidebar`(导航目录) / `PageHost2`(多页宿主: preview child + 选中页 layout, pages 为 `std::span<Node* const>`) / `SectionHead`(标题+换行说明+分隔) / `CodeBlock`(等宽+行号+语法着色+Copy, `codehl::scanLine` 扫描器, `ImFont* mono` 注入) / `FrameLabel` / `ClosePopupButton` 

**wg conventions**: state lives OUTSIDE (controls bind `float*`/`bool*`/`int*` — default-constructible so tuple children work); `items` arrays must outlive run(); Control is non-copyable (object address = ImGui ID); decorators push/pop in renderImpl; template children default-constructible required (Control's default ctor is `requires`-guarded on all children); splits are fill-type (`Node::fill`), Fixed sizes are design px (em-scaled), `minPx`/`maxFrac` constrain shrinking — never `std::clamp` with lo>hi (hidden Whens arrange at 0-size rects; use `detail::clampCellValue`). Rendering is origin-propagated (`Node::origin`): same-space containers use `detail::drawChildAt`, flow contexts (popup/menu/tab/decorators) use `drawChildFlow`, window-space containers (SubWindow/Panel) use `drawChildLocal` — nesting containers/splits inside child windows keeps coordinates correct; when embedding a whole root tree inside a control, pass `node.origin` as the layout host origin (see gallery PageHost).
Runtime `node.add()` during render (onClick etc.) is **deferred**: goes to `Node::m_Pending`, applied by `flushPending()` at the start of the node's next measure/arrange/draw/layout — the render-depth guard is the thread-global `detail::g_RenderDepth` (NOT per-node), so adding to a container whose own `layout()` is never called (e.g. a Layout nested under another) is still safe. `remove()` tombstones in `children` AND scrubs `m_Pending` (covers add-then-remove within the same frame). Never push_back into `children` directly during render (iterator invalidation = crash).

## Layout

- `imgui_mk/` is effectively header-only: all logic in `include/` (`imgui_mk/{window.h,mkui.h,windowRendereren.h,Mainwindow.h}`, `imgui_mk/ui/{wg.h,wg_controls.h,wg_style.h,wg_file.h}`, `string_mk/mk_string.h`); `src/imgui_mk.cpp` is empty. Both include styles (`<imgui_mk/window.h>` and flat `<Mainwindow.h>`) are used.
- `window.h` uses `wg::Theme` for the adaptive-font machinery (`setTheme/rescale/setAutoScaleFont/setFontLoader`, `Impl::doRescale/processAdaptiveScaling`). `setWidget()` was removed — mount wg trees via `setUserUI`/`setUI` + `tree.layout(rect)` per frame.
- CMake uses `file(GLOB_RECURSE ... CONFIGURE_DEPENDS)` for every target, so new files need no CMake edits, but deleted files must be removed from disk or the glob still picks them up.
- `apps/` = `gallery` (wg 全量画廊, 22 pages, dual window + FileDialog + theme; **Qt 风格组装**: `GalleryWindow`(构造组装窗口) + `addPage(page, group)` 注册页面 + `refreshAll` 刷新; 页面 `PageXxx(State&)` 构造函数添加控件(`page.addWidget`), `page.setup(标题/说明/snippet)` 页铬; **按页面分文件**: `gallery_state.h`(State/FnDrawerImpl/gMono), `gallery_chrome.h`(页架 Page + InsUi), `gallery_pagehost.h`(gallery 专属 PageHost 页面宿主/PageNav 页面导航, addPage/addEntry 自持集合), `pages/page_XX_*.h`(22 页各一文件), `gallery_pages.h`(include 聚合 + kGroupXxx 分组常量), `gallery_app.h`(GalleryWindow)). passman was removed.
- `tutorials/` = 13 tutorial programs + lessons docs, all wg-based (course table in `tutorials/README.md`).
- `test/` is defined by the single `test/CMakeLists.txt` (flat): GUI examples `test`/`test2`/`test3` (engine smoke) + `test16..test20` (wg verification, auto-close, exit code 0 = pass: 16 layout/lifecycle asserts, 17 full-control soak, 18 dual-window link, 19 theme/rescale, 20 split/em asserts), plus gtest `unit_tests` globbed from `test/*_test.cpp` (`mkui_test`, `mk_string_test`, `window_test`, `wg_test` — wg_test is ImGui-free pure logic). The unit glob matches `*_test.cpp` only — keep it that way so `windowstest*.cpp` examples are not pulled into `unit_tests` (duplicate `main`).

## Build

- Requires CMake 4.1.1+, C++23, and a compiler supporting experimental module import std (root `CMakeLists.txt` sets `CMAKE_EXPERIMENTAL_CXX_IMPORT_STD` with a magic GUID — do not change it). Practically MSVC.
- `find_package(glfw3 REQUIRED)` and `find_package(Assimp REQUIRED)` are called by `imgui/CMakeLists.txt`; configure fails if these aren't findable (installed at `C:\Program Files\glfw3` and `C:\Program Files (x86)\Assimp`).
- Build trees: `out/build/x64-Debug` (Ninja, `-DCMAKE_BUILD_TYPE=Debug`, CLI-driven) and `out/build/x64-release` (Release); `out/` is gitignored. The old VS "Open Folder" tree was recreated as plain Ninja — reconfigure via:
  ```
  cmake -S . -B out/build/x64-Debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
  ```
- CLI builds need an MSVC env: wrap commands with `VsDevCmd.bat -arch=amd64` (cl.exe is not on PATH otherwise). Ninja workflow (verified):
  ```
  cmd /c "\"C:\Program Files\Microsoft Visual Studio\18\Enterprise\Common7\Tools\VsDevCmd.bat\" -arch=amd64 >nul && cmake --build out/build/x64-Debug --target test16 test17 unit_tests gallery"
  ```
- **Unit tests (gtest + CTest)**: gtest is fetched from GitHub on first configure and built per-tree, so it matches the tree's CRT/ASan flags — never mix it with prebuilt gtest libs (LNK2038). Run with `ctest --test-dir out/build/x64-Debug/test --output-on-failure` (CTest is enabled in the `test/` subdir, NOT at root — CMake 4 reserves the target name `test`, which the example target uses).
- There is no `test/run_tests.ps1` anymore; soak GUI examples manually (launch, then WM_CLOSE; auto-close tests exit 0).

## Gotchas

- Global engine is a template, header-only in `mkui.h`: `Mk::mkui<WindowT>::RegistrationWindow()` (no-arg) is a Meyers-singleton accessor; the engine stores `std::vector<std::shared_ptr<WindowT::Impl>>` and `Registration(std::shared_ptr<ImplT>)` shares the window state instead of taking it away. One engine instance per window type; template function-local statics are merged across TUs, so the registry is safe in multi-TU projects (unlike a plain header `static`).
- Shared ownership, not transfer: `Mk::window` is a thin handle wrapping `shared_ptr<Impl>`; the constructor registers that same `shared_ptr` into the engine. **Closing is real**: clicking X or `Mk::window::close()` destroys the GLFW window + ImGui context immediately; every shared handle then reports `getHandle() == nullptr`, and `shouldClose()` stays true so the engine drops the window. Impl contract: `render()`, `shouldClose()`, `close()`. GLFW callbacks are static methods of `window::Impl` routed via `glfwGetWindowUserPointer`, and every callback must call `ImGui::SetCurrentContext(impl->m_ImGuiContext)` first.
- `Impl::close()` shutdown order is load-bearing: ImGui's GLFW backend hooks a WndProc into every window on Windows, and that hook reads the *current context's* backend data with no null guard. `glfwDestroyWindow` synchronously delivers activation messages to still-live windows, so the GLFW backend must NOT be shut down until *after* `glfwDestroyWindow` (renderer backend first, then `ImGui_ImplGlfw_RestoreCallbacks`, destroy window, then `ImGui_ImplGlfw_Shutdown` + `ImGui::DestroyContext`). Reorder at your peril.
- Minimized windows: `Impl::render()` (and `doRescale()`) early-return when framebuffer size <= 0 — a 0-size frame corrupts the ImGui window/ID stack (user-visible as `PopID() too many times!` assert in a child window). Never render UI on a 0x0 framebuffer.
- Renderer is selected by macros at the top of `windowRendereren.h`; only `USE_RENDERER_OPENGL` is implemented (`Rendererengine<opengl, int>`). DX/Vulkan are empty stubs.
- UI content: `window::setUserUI(std::function<void()>)` injects per-frame UI (called inside the fullscreen ImGui window in `Impl::render()`); the callback must outlive the engine's run loop. `Mainwindow.h` provides `Mk::MainWindow<WindowT>` — constructs + auto-registers a window (default 1600x900) and owns the source object; it has NO UI of its own. **`setCentralWidget(Node&, padX, padY)`** mounts the central wg tree — the engine auto-layouts it every frame AFTER the setUserUI callback (layout driven by window⇄control-tree interaction, Qt setCentralWidget semantics; apps make zero layout calls). 树原点默认 = 样式 WindowPadding（外边距主题联动），padX/padY 为其增量；布局层默认 margin 会叠加，主布局防叠加需 setContentsMargins(0,0)（见 gallery_app.h）。
- Comments are in Chinese; source must stay UTF-8 (MSVC `/utf-8`; non-MSVC `-finput-charset=utf-8`). Keep the Chinese comments.
- `mk::string<N>` (`string_mk/mk_string.h`) is a custom fixed-capacity string used by the examples, with quirky constructors — don't replace it with `std::string` without checking the examples that use it.



