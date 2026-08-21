# 第 11 课：异步文件对话框

wg2 直接复用旧 FileDialog 实现：Mk::wg2::FileDialog。

## 11.1 打开对话框

`cpp
Mk::wg2::FileDialog dlg;

dlg.open("Open File", {
    {"Text/Code", "*.txt;*.md;*.cpp;*.h"},
    {"All files", "*.*"},
});
`

## 11.2 每帧轮询

`cpp
std::string path;
if (dlg.poll(path)) {
    if (path.empty()) { /* user cancel */ }
    else { /* load */ }
}
`

## 11.3 生命周期

- worker 线程，主循环不冻结
- busy 期间重复 open() 是空操作
- 对象应覆盖 run() 生命周期

## 练习

在页面里加选文件按钮和路径显示。
