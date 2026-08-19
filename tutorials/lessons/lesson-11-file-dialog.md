# 第 11 课：异步文件对话框

配套程序：`src/tutorial11_filedialog.cpp`。`Mk::wg::FileDialog` 是
Win32 IFileOpenDialog 的异步封装：对话框跑在 worker 线程，主循环
不阻塞；每帧 `poll()` 取结果。

## §11.1 长期存活

```cpp
Mk::wg::FileDialog dlg;   // 存活期覆盖 run()（析构等待对话框关闭）
```

对话框打开期间重复 `open()` 为空操作；对象析构会 join worker 线程
（用户关掉对话框才返回），因此它必须活到 `run()` 结束。

## §11.2 启动与轮询

```cpp
dlg.open("打开文件（教程11）", {
    {"文本/代码", "*.txt;*.md;*.cpp;*.h;*.json;*.ini;*.log"},
    {"所有文件", "*.*"},
});

std::string picked;
if(dlg.poll(picked)) {        // 完成返回 true（取消时 picked 为空）
    path = picked;
    ...
}
```

`busy()` 指示对话框是否打开中——配套程序用它做"打开中…"的条件提示。

## §11.3 主循环不冻结的证明

配套程序在帧循环里持续驱动一条正弦进度条：对话框打开期间它仍在动——
证明 worker 线程与主渲染循环互不阻塞。

## §11.4 结果处理：文本/二进制判定

选完文件后读取前 64KB，按内容判定：

```cpp
size = std::filesystem::file_size(path, ec);
std::ifstream in(path, std::ios::binary);
std::string buf(64 * 1024, '\0');
in.read(buf.data(), (std::streamsize)buf.size());
buf.resize((size_t)in.gcount());
binary = buf.find('\0') != std::string::npos;   // 含 NUL = 二进制
if(!binary) preview = std::move(buf);            // 文本进预览
```

二进制只显示大小；文本进 `SubWindow` 预览（`TextUnformatted` 原样含
换行）。路径/大小/条件全部经 wg 控件的绑定指针与 When 展示。

## 练习

1. 把过滤器改成图片（`*.png;*.jpg`），预览改为显示文件大小与字节数。
2. 给 `FileDialog` 加第二个实例：一个"打开"，一个"另存为"（思路：
   `open` 是唯一的启动点，封装一层即可）。
3. 观察：对话框打开时拖动主窗口，确认窗口仍能正常重绘。
