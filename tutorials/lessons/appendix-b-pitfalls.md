# 附录 B：常见陷阱 FAQ（mkui 开发踩坑实录）

本附录收录 mkui 库开发过程中**真实发生并修复**的问题——每条都是
一段可复现的历史。读它等于少走我们走过的弯路。

## 1. 多窗口同时关闭崩溃（WndProc 钩子竞态）

**症状**：两个窗口同帧收到关闭 → `0xC0000005` 访问违例（事件日志
异常码 `0xC000041d`，回调中崩溃）。

**根因**：ImGui 的 GLFW 后端在 Windows 上把 WndProc 钩子装到每个窗口
上（`ImGui_ImplGlfw_WndProc`），钩子取**当前上下文**的后端数据且无空
指针保护。窗口 A 销毁完 ImGui 上下文后当前上下文变 NULL，窗口 B 的
钩子仍装着——此时任何消息到达 B 就解引用空指针。

**修复**（引擎层）：关闭窗口后立即把 ImGui 当前上下文切到任一存活
窗口；轮询前若当前上下文为空也补设（覆盖循环外手动 close 的场景）。
用 `requires` 能力探测，不破坏最小 Impl 契约（FakeWindow 单测仍在跑）。

**教训**：跨窗口的系统级副作用，永远要在"上下文正确"的时机处理。

## 2. UI 回调内直接 close() 崩溃

**症状**：按钮回调里 `window.close()` → 崩溃。

**根因**：`close()` 同步销毁 ImGui 上下文，而本帧渲染尚未结束——
后续 `ImGui::End()/Render()` 操作已销毁的上下文。

**修复**：回调里用 `glfwSetWindowShouldClose(handle, true)` 请求关闭，
引擎在帧间安全时机真销毁（第 01 课 §1.5 口诀）。

## 3. 菜单栏与宿主内容重叠

**症状**：顶部菜单栏压住下方 UI 约一个菜单条高度。

**根因**：`BeginMainMenuBar` 是浮在视口顶部的**独立窗口**，不占宿主
布局空间——宿主内容从顶部排起，与菜单栏重叠。

**修复**：`MenuBar` 的 renderImpl 在 `EndMainMenuBar` 后把宿主光标下移
`GetFrameHeight()`（内建处理）；约定：菜单栏放窗口顶部独立布局
（`menuTree.layout({0, 0, w, 24})`）。

## 4. 字号档位/DPI 下 fixed 尺寸失调（em 记账错位）

**症状**：`rescale(27)` 后 stretch 格越界、右端被裁。

**根因**：旧版 `HSplit` 的 `fixedSum` 用**设计值**记账，`drawCell`
渲染时再乘 em 因子——字号 >18 时账目错位，stretch 超发。

**修复**：约束/收缩/em 全在分格 arrange 里算好，子格只接收实际像素，
账目同源；`test20` 用 `fixed+stretch+gap == avail` 断言锁死。

## 5. 侧边栏吃半屏（fixed 不可收缩）

**症状**：窗口拖窄时固定侧栏占比越来越大，最多吃掉 50%+。

**修复**：flexbox 式三层约束 `.min(px).atMost(f)`——min 管文字底线、
preferred 管比例、atMost 管礼让；空间不足按 shrink 系数向 min 收缩
（第 07 课）。CSS 同语义：min-width 优先于 max-width。

## 6. 字体重烘焙断言（ClearFonts 不清配置）

**症状**：`rescale` 第二次重烘焙时 ImGui 断言
`cfg.DstFont && (!cfg.DstFont->IsLoaded() || ...)`。

**根因**：`io.Fonts->ClearFonts()` 只清字体列表，**ConfigData 残留**，
重复 `AddFont*` 触发断言。

**修复**：用 `io.Fonts->Clear()`（全量重置含配置）——窗口.h 的
`doRescale` 现注释写明。

## 7. 渲染中立即毁纹理 → 本帧悬垂

**症状**：UI 回调内调 rescale → 本帧 RenderDrawData 引用已删除的
GL 纹理。

**修复**：rescale 改为**请求制**——`requestRescale` 只置标志，render
顶部（NewFrame 前）统一执行（`m_PendingRescale`）。公共 API 仍是
`rescale()`，语义"下一帧生效"。

## 8. mk_string 的连环 bug（顺带体检的教训）

**症状/根因**（重写前）：`sizeof(指针)` 当容量判断（恒 8）、
`snprintf(buf, N, str)` 把内容当格式串（%s/%n 即 UB）、移动构造只搬
`string_view`（悬垂且没拷字符）、跨容量按对方缓冲区容量误判。

**修复**：不变量重构 `m_Buf[N]+m_Size`（恒 NUL 结尾）、memcpy 语义、
`clampedLen` 限长扫描、模板跨容量拷贝。**教训**：字符串类优先
`std::string`/`std::array<char,N>`；自造轮子前先做 code review。

## 9. 变量名遮蔽控件类型

**症状**：`Mk::wg::Button button;` 后出现"非类型模板参数"等诡异报错。

**根因**：局部变量/参数名与 wg 类型同名（如 `Label label;` 中
`Label` 被变量遮蔽，后续 `Mk::wg::Label` 仍是类型但裸 `Label`
指变量），模板实参解析错乱。

**修复**：变量用语义名（`lbl`/`btn`/`slideV`），或用全限定
`Mk::wg::Label`。wg 的 `Control` 禁拷贝也意味着别给控件起别名变量。

## 10. 临时 span / 临时视图悬垂

**症状**：`textTable("t", span{"名","值"}, rows)` 偶发花屏/乱码。

**根因**：init-list 构造的临时 span 指向临时数组，帧结束即悬垂
（第 06 课 §6.6-1）。

**修复**：items/headers 用 static 或长期存活数组。教训：**凡是控件
"持有指针/引用"的参数，调用方负责存活期**——这是贯穿 mkui 的
统一契约。

## 11. Popup 与 Modal 的独占语义

**症状**：普通弹窗与模态弹窗同时打开，普通弹窗消失。

**根因**：ImGui 模态独占——`OpenPopup` 模态会强制关闭已打开的其他
弹窗（test17 用帧计数固化了该行为：普通弹窗 60 帧后被模态顶掉）。

**教训**：模态与普通弹窗不要设计成"同时常驻"。

## 12. 立即模式的"想当然"

- **每帧回调不是事件处理**：所有代码每帧执行，重活放线程（第 02 课）；
- **状态不自动保留**：控件无记忆，状态全在绑定变量里（第 02 课）；
- **文本不是"更新"而是"重画"**：改控件数据成员（`setText`）即生效，
  没有"推送"心智——wg Label 的 setText 也只是改内部字符串。

## 通用排查流程

1. 先跑 `test16..20` 与 `ctest`（每例断言一个特性，快速定位回归面）；
2. GUI 视觉问题：对照 gallery 同功能页（它是全 API 的活样例）；
3. 崩溃：优先怀疑"存活期契约"（指针/引用参数）；ASan 树复现；
4. 布局异常：检查 em/约束/字号档位三者的交互（第 07/08 课）。
