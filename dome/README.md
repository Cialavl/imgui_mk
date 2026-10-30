# dome —— 个人验证 demo 目录
#
# 与 `test/` 不同：这里不是自动化测试，而是用来手动运行、
# 验证 AI 生成代码、做快速实验的独立小应用。

## 用法

```bash
cmake --build build --target dome --config Debug
./build/dome/Debug/dome.exe
```

或直接手动编译 `src/main.cpp` 并链接 `imgui_mk`。
