# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**imgui_mk** is a C++23 library that provides a wrapper around ImGui (Immediate Mode GUI) for creating simple windowed applications with OpenGL rendering. It simplifies ImGui window management, input handling, and rendering through a convenient API.

## Build System

This project uses **CMake** (minimum version 4.1.1) with C++23 modules enabled.

### Building the Project

```bash
# From the root directory
mkdir build
cd build

# Configure with CMake (Ninja is recommended)
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..

# Build all targets
cmake --build .

# Or build a specific target
cmake --build . --target imgui_mk
cmake --build . --target test
```

### Build Outputs

The project produces:
- `imgui_mk` - Main static library
- `test` - Test executable (`test/windowstest.cpp`)
- `test2` - Additional test executable

### Running Tests

```bash
# From the build directory
./test/windowstest
# or on Windows:
test\windowstest.exe
```

## Architecture

The library is organized around a central window registry pattern with a modular renderer architecture:

### Core Components

- **`Mk::window`** (`imgui_mk/include/imgui_mk/window.h`): Main window class that combines GLFW, OpenGL, and ImGui. Each window has its own ImGui context and manages input callbacks through static functions with user pointer routing.

- **`Mk::mkui`** (`imgui_mk/include/imgui_mk/mkui.h`): Central UI runner that maintains a static registry of windows. Call `mkui::run()` to start the event loop, which automatically handles window lifecycle (creation, rendering, and destruction).

- **`Rendererengine<Renderer, Args...>`** (`imgui_mk/include/imgui_mk/windowRendereren.h`): Template-based renderer abstraction. Currently only OpenGL is implemented (`Rendererengine<opengl, int>`), but the architecture is designed to support DirectX and Vulkan in the future.

### Window Lifecycle

1. Create a `Mk::window` instance with title, width, height
2. Register it with `Mk::mkui::RegistrationWindow(&window_ptr)`
3. Call `Mk::mkui::run()` to start the main loop
4. Windows automatically render and clean up when closed

### Multi-Window Pattern

Each window maintains its own:
- GLFW window handle
- ImGui context (isolated from other windows)
- Input callbacks (routed through static methods with user pointer)

This allows multiple independent windows to coexist in the same application.

## Dependencies

### Required External Libraries
- **GLFW 3** - Window and input management (found via `find_package(glfw3)`)
- **GLAD** - OpenGL loader (included as submodule in `glad/`)
- **ImGui** - Immediate mode GUI library (included as submodule in `imgui/`)

### Build Requirements
- C++23 compatible compiler (MSVC on Windows, GCC/Clang on Linux/macOS)
- CMake 4.1.1+
- Ninja build system (recommended)

### Optional
- **Assimp** - Asset Import library (required by CMake but not actively used)

## Project Structure

```
imgui_mk/
├── imgui_mk/          # Main library (public headers in include/, sources in src/)
├── imgui/             # ImGui submodule
├── glad/              # GLAD OpenGL loader submodule
├── test/              # Test executables
└── build/             # Build output directory (generated)
```

## Development Notes

- The project uses experimental C++ modules (`CMAKE_EXPERIMENTAL_CXX_IMPORT_STD`)
- Source code contains Chinese comments
- Windows-specific MSVC flags enabled: `/Zc:preprocessor /utf-8 /DNOMINMAX /D_USE_MATH_DEFINES /EHsc /bigobj`
- On non-MSVC Windows: `-finput-charset=utf-8 -fexec-charset=utf-8` for UTF-8 support
- The renderer can be switched by editing `windowRendereren.h` (uncomment `USE_RENDERER_DX` or `USE_RENDERER_VULKAN`)
