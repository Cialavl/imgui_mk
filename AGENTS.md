# AGENTS.md

C++23 GUI library wrapping ImGui + GLFW + OpenGL. Windows-focused. See `CLAUDE.md` for architecture prose, but note it is partly stale: `imgui/` and `glad/` are **vendored in-tree, not git submodules** (no `.gitmodules`), and testing lives in `test/` (GUI examples `test/windowstest*.cpp`, unit tests `test/*_test.cpp`).
- `glad/` and `imgui/` are marked read-only on disk (all files IsReadOnly). Don't edit vendored code; if you must, clear with `attrib -R <file>` (filesystem attribute only, git ignores it).

## Build

- Requires CMake 4.1.1+, C++23, and a compiler supporting experimental module import std (root `CMakeLists.txt` sets `CMAKE_EXPERIMENTAL_CXX_IMPORT_STD` with a magic GUID — do not change it). Practically MSVC.
- `find_package(glfw3 REQUIRED)` and `find_package(Assimp REQUIRED)` are called by `imgui/CMakeLists.txt`; configure fails if these aren't findable (they come from the environment, e.g. vcpkg).
- Existing build trees: `build/` (Visual Studio 18 2026, multi-config) and `out/build/x64-Debug` / `x64-release` (Ninja, from VS "Open Folder"; `out/` is gitignored). ASan tree: `out/build/x64-asan` (configured with /fsanitize=address).
- Ninja workflow (verified):
  ```
  cmake --build out/build/x64-Debug --target test test2 test3
  out/build/x64-Debug/test/test.exe
  ```
- **Unit tests (gtest + CTest)**: gtest is fetched from GitHub on first configure and built per-tree, so it matches the tree's CRT/ASan flags — never mix it with prebuilt gtest libs (LNK2038). Run with `ctest --test-dir out/build/x64-Debug/test --output-on-failure` (CTest is enabled in the `test/` subdir, NOT at root — CMake 4 reserves the target name `test`, which the example target uses).
- Automated GUI smoke tests: `pwsh test/run_tests.ps1` — runs ctest (unit tests) first, then soaks each example exe (500ms crash polling), sends WM_CLOSE to all visible windows, asserts clean exit code 0. `-Asan` runs the ASan build, `-Build` builds first (auto-configures missing trees, incl. ASan flags), `-SoakSeconds` / `-Iterations` tune rigor. The script auto-copies the ASan runtime DLL next to each exe.

## Layout

- `imgui_mk/` is effectively header-only: all logic is in `include/` (`imgui_mk/{window.h,mkui.h,windowRendereren.h}`, plus flat `Mainwindow.h` and `string_mk/mk_string.h`); `src/imgui_mk.cpp` is empty. Both include styles (`<imgui_mk/window.h>` and flat `<Mainwindow.h>`) are used.
- CMake uses `file(GLOB_RECURSE ... CONFIGURE_DEPENDS)` for every target, so new files need no CMake edits, but deleted files must be removed from disk or the glob still picks them up.
- Everything under `test/` is defined by the single `test/CMakeLists.txt` (flat, no subdirectories): GUI example targets `test` / `test2` / `test3` from `windowstest.cpp` / `windowstest2.cpp` / `windowstest3.cpp`, plus the gtest `unit_tests` target globbed from `test/*_test.cpp`. Exes land in `out/build/<cfg>/test/`. The unit glob matches `*_test.cpp` only — keep it that way so the examples' windowstest*.cpp files are not pulled into `unit_tests` (duplicate `main`).
- `test` (windowstest.cpp) grabs the engine with `Mk::mkui<Mk::window>::RegistrationWindow()`, creates windows via `std::make_unique<Mk::window>(...)` (auto-registered, no .release()) and calls `Mk::mkui<Mk::window>::run()`. `test2`/`test3` use the `Mk::MainWindow<Mk::window>` component (see Gotchas).

## Gotchas

- Global engine is a template, header-only in `mkui.h`: `Mk::mkui<WindowT>::RegistrationWindow()` (no-arg) is a Meyers-singleton accessor; the engine stores `std::vector<std::shared_ptr<WindowT::Impl>>` and `Registration(std::shared_ptr<ImplT>)` shares the window state instead of taking it away. One engine instance per window type; template function-local statics are merged across TUs, so the registry is safe in multi-TU projects (unlike a plain header `static`).
- Shared ownership, not transfer: `Mk::window` is a thin handle wrapping `shared_ptr<Impl>`; the constructor registers that same `shared_ptr` into the engine. So `Mk::window("title", w, h);` (bare temporary), `auto w = std::make_unique<Mk::window>(...)` and named locals all work — no .release(), and the source object keeps working after construction and after `run()` (the `Impl` object lives as long as any `shared_ptr`). **Closing is real**: clicking X (engine run loop) or `Mk::window::close()` destroys the GLFW window + ImGui context immediately, regardless of how many `shared_ptr`s remain — every shared handle then reports `getHandle() == nullptr`, and `shouldClose()` stays true so the engine drops the window from its registry. Impl contract: `render()`, `shouldClose()`, `close()`. The GLFW callbacks are static methods of `window::Impl`, routed via `glfwGetWindowUserPointer`.
- `Mk::window` (window.h): each window has its own ImGui context. GLFW input callbacks are static methods routed via `glfwGetWindowUserPointer`, and every callback must call `ImGui::SetCurrentContext(impl->m_ImGuiContext)` first. Follow this pattern when adding callbacks.
- `Impl::close()` shutdown order is load-bearing: ImGui's GLFW backend hooks a WndProc into every window on Windows, and that hook reads the *current context's* backend data with no null guard. `glfwDestroyWindow` synchronously delivers activation messages to still-live windows, so the GLFW backend must NOT be shut down until *after* `glfwDestroyWindow` (renderer backend first, then `ImGui_ImplGlfw_RestoreCallbacks`, destroy window, then `ImGui_ImplGlfw_Shutdown` + `ImGui::DestroyContext`). Reorder at your peril.
- Renderer is selected by macros at the top of `windowRendereren.h`; only `USE_RENDERER_OPENGL` is implemented (`Rendererengine<opengl, int>`). DX/Vulkan are empty stubs.
- UI content: `window::setUserUI(std::function<void()>)` injects per-frame UI (called inside the fullscreen ImGui window in `Impl::render()`); the callback must outlive the engine's run loop. `Mainwindow.h` provides `Mk::MainWindow<WindowT>` — a specialized window component: constructs + auto-registers a window (default 1600x900) and owns the source object; it has NO UI of its own.
- Comments are in Chinese; source must stay UTF-8 (MSVC `/utf-8`; non-MSVC `-finput-charset=utf-8`). Keep the Chinese comments.
- `mk::string<N>` (`string_mk/mk_string.h`) is a custom fixed-capacity string used by the examples, with quirky constructors — don't replace it with `std::string` without checking the examples that use it.