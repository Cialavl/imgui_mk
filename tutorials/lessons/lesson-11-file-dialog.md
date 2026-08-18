# 第 11 课：本地文件对话框——异步状态机

配套程序：`src/tutorial11_filedialog.cpp`——打开文件、读前 64KB、预览。
本课核心不是"怎么打开对话框"，而是**异步模式**：对话框跑在 worker
线程，主渲染循环不冻结。

## §11.1 为什么必须异步

Win32 文件对话框（IFileOpenDialog）是模态的——主线程调用它会阻塞
消息泵。若直接在 setUI 里调用：渲染循环卡死、窗口变"无响应"、
所有窗口冻结（多窗口时连别的窗口也拖不动）。

mkui 的 `FileDialog` 把对话框放**专用 worker 线程**（线程内自备 STA
COM），主线程只做两件事：发起请求 + 每帧轮询结果。

## §11.2 状态机与 API

```text
Idle ──open()──▶ Opening ──用户选完/取消──▶ Done ──poll()──▶ Idle
                （busy() 为 true）          （结果可取走）
```

| API | 语义 |
|---|---|
| `open(title, filters)` | 发起异步对话框；busy 中重复调用为空操作 |
| `busy()` | 对话框是否在途（UI 可显示"打开中…"） |
| `poll(path)` | 完成则返回 true 并取走结果（**取消 = 空串**），状态回 Idle |

```cpp
Mk::fui::FileDialog dlg;                    // 长期存活（成员/栈，活过 run()）
dlg.open("打开文件", {{"文本", "*.txt;*.md"}, {"所有文件", "*.*"}});
// ……主循环继续渲染……
std::string path;
if(dlg.poll(path) && !path.empty()) { /* 装载文件 */ }
```

配套程序 §11.2 区：按钮发起 → 对话框弹出期间**主窗口动画照跑**
（进度条正弦驱动）→ 选文件后路径/大小/预览显示；点取消则空串。

## §11.3 worker 线程细节（了解即可）

- 对话框线程内 `CoInitializeEx(STA)` 起家、`CoUninitialize` 收尾；
- 结果用 `std::string` 写回，`poll` 消费时 join 线程；
- **UI 线程永不触碰对话框线程对象**，反之亦然——状态机 + 每帧 poll
  是唯一通信通道；
- 析构会等待对话框关闭（用户操作后才返回）——`FileDialog` 对象必须
  活过 run()（它可能在 run 期间被 open）。

## §11.4 装载文本预览（配套程序读文件部分）

```cpp
std::error_code ec;
size = std::filesystem::file_size(path, ec);
if(ec) { /* 不可读 */ }
else {
    std::ifstream in(path, std::ios::binary);
    std::string buf(64*1024, '\0');
    in.read(buf.data(), (std::streamsize)buf.size());
    buf.resize((size_t)in.gcount());          // 实际读到的字节
    binary = buf.find('\0') != std::string::npos;  // 含 NUL 判定二进制
    if(!binary) preview = std::move(buf);
}
```

读取发生在 **poll 返回后、主线程**（对话框线程只负责选文件）——
文件 IO 在主线程做一次可接受；更大文件可再起线程（本课不展开）。

## §11.5 常见错误

1. **FileDialog 是局部临时**：`open` 后对象析构 → join 等对话框关闭，
   但 poll 无处可取。长期存活。
2. **忙时重复 open**：状态机已兜底（空操作）✓ 无需自己判断。
3. **在对话框线程访问 UI 状态**：没有 API 暴露这种通道，遵守状态机即可。
4. **把 poll 当"同步等待"**：poll 只查一次，未完成返回 false——每帧
   调一次，配合 UI 显示 busy 状态才是正确节奏。

## §11.6 练习

1. 配套程序：把预览区换成只读 `inputTextMultiline`（多行显示更舒服）。
2. 增加"另存为"按钮：`IFileSaveDialog` 风格（可查 fui_file.h 扩展，
   或先只用 open 演示）。
3. 挑战：选中 .cpp 文件时自动统计行数显示在状态栏（读取在 poll 后
   主线程，注意大文件性能）。

## §11.7 进一步阅读

- `imgui_mk/include/imgui_mk/ui/fui_file.h`：完整实现（worker 线程 +
  状态机 + COM 封装，约 150 行）。
- `apps/gallery` "Open local file" 页 + Files 页的 tooltip(labelEllipsis)。
- 下一课（完结）：扩展你自己的控件——ADL 三步法与自定义字体链。