# wg2 测试体系（一个文件 = 一个验证点，内含多个用例）

组织方式：
- **一个 `.cpp` 只负责一个验证点**（一个控件族 / 一个容器 / 一类行为），文件名即主题；
- 文件内可以有多个 `TEST`，分别覆盖这个点的不同角度（如 VBox 的堆叠、边距、固定高度、裁剪、hint）；
- 公共夹具集中在 `wg2_test_harness.h`：离线 ImGui 上下文 `Harness`、位置探针 `RectProbe`/`CaptureProbe`、
  选项表 `kItems`、引擎假窗口（`LoopWindow`/`BroadcastWindow`/`MinimalWindow`）、UTF-8 校验。

原 `wg2_smoke.cpp`、`wg2_test.cpp`、`mkui_test.cpp` 的内容已并入下表各文件后删除。

## 第 0 层：基础类型

| 文件 | 用例 | 验证点 |
|---|---|---|
| `wg2_core_test.cpp` | 3 | 基础类型：编译期字符串 ID、字符串路径访问、尺寸策略 |

## 第一层：单个组件

| 文件 | 用例 | 验证点 |
|---|---|---|
| `wg2_components_basic_test.cpp` | 3 |  |
| `wg2_components_input_test.cpp` | 1 |  |
| `wg2_components_display_test.cpp` | 1 |  |
| `wg2_components_color_test.cpp` | 1 |  |
| `wg2_components_decorator_test.cpp` | 1 |  |
| `wg2_components_menu_popup_test.cpp` | 2 |  |
| `wg2_components_table_test.cpp` | 2 |  |
| `wg2_components_text_ellipsis_test.cpp` | 2 |  |

## 第二层：布局组合

| 文件 | 用例 | 验证点 |
|---|---|---|
| `wg2_layout_vbox_test.cpp` | 5 |  |
| `wg2_layout_hbox_test.cpp` | 3 |  |
| `wg2_layout_grid_wrap_test.cpp` | 4 |  |
| `wg2_layout_nested_test.cpp` | 5 |  |

## 第三层：消息循环

| 文件 | 用例 | 验证点 |
|---|---|---|
| `wg2_loop_input_test.cpp` | 3 |  |
| `wg2_loop_frame_test.cpp` | 4 |  |

## 第四层：组件间通信

| 文件 | 用例 | 验证点 |
|---|---|---|
| `wg2_comm_binding_test.cpp` | 3 |  |
| `wg2_comm_dynamic_test.cpp` | 5 |  |
| `wg2_comm_engine_test.cpp` | 9 |  |

## 其它模块

| 文件 | 用例 | 验证点 |
|---|---|---|
| `mk_string_test.cpp` | 17 | `Mk::string_mk` 的构造 / 拷贝 / 赋值 / 容量边界 |

## 运行

```bash
cmake --build out/build/x64-Debug --target unit_tests
out\build\x64-Debug\test\unit_tests.exe --gtest_filter=Wg2Layout.*   # 只跑某一层
ctest --test-dir out/build/x64-Debug/test -R Wg2                       # 按层跑
ctest --test-dir out/build/x64-Debug/test                              # 全部（6 项）
```

## 覆盖情况

**结构性覆盖（已验证）**：wg2 公开控件/容器共 **59 个类型全部被测试引用并绘制**，动态容器与引擎层 API 也都有对应用例。

**行覆盖率**：本机没有可用的覆盖率工具（无 OpenCppCoverage / llvm-cov / gcov；VS 自带的 `CodeCoverage.exe` 已是停用壳）。装好工具后可直接量：

```powershell
# 方案 A：OpenCppCoverage
OpenCppCoverage --sources ...\imgui_mk\include\imgui_mk\ui\wg2 --export_type=html:cov -- out\build\x64-Debug\test\unit_tests.exe

# 方案 B：clang-cl + llvm-cov
cmake -S . -B out/build/cov -G Ninja -DCMAKE_BUILD_TYPE=Debug ^
      -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_C_COMPILER=clang-cl ^
      -DCMAKE_CXX_FLAGS="-fprofile-instr-generate -fcoverage-mapping" ^
      -DCMAKE_EXE_LINKER_FLAGS="-fprofile-instr-generate"
cmake --build out/build/cov --target unit_tests
set LLVM_PROFILE_FILE=cov\wg2-%p.profraw
out\build\cov\test\unit_tests.exe
llvm-profdata merge -sparse cov\*.profraw -o cov\wg2.profdata
llvm-cov report out\build\cov\test\unit_tests.exe -instr-profile=cov\wg2.profdata -sources imgui_mk\include\imgui_mk\ui\wg2
```

**已知不可离线覆盖**（需真实 GL/窗口/用户交互）：`texture.h`、`window.h` 的真实窗口与 DPI 重烘焙、`FileDialog` 打开后的交互、`mkui::run()` 的真实窗口循环（用假窗口类型覆盖了调度逻辑）。
