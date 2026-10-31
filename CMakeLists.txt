cmake_minimum_required(VERSION 4.1.1)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(imgui_mk LANGUAGES  CXX)

if (MSVC)
    add_compile_options(/Zc:preprocessor /utf-8 /DNOMINMAX /D_USE_MATH_DEFINES /EHsc /bigobj)

    # ninja 靠解析 cl 的 /showIncludes 输出建依赖表；中文版 VS 的输出前缀是
    # "注意: 包含文件: "，不匹配会导致依赖表为空（改头文件不触发重编）。
    # 英文版 VS 上用 -DCMAKE_CL_SHOWINCLUDES_PREFIX="Note: including file: " 覆盖。
    if (NOT DEFINED CMAKE_CL_SHOWINCLUDES_PREFIX)
        set(CMAKE_CL_SHOWINCLUDES_PREFIX "注意: 包含文件: " CACHE STRING "cl /showIncludes 前缀（随语言本地化）")
    endif()
else()
    if (WIN32)
        add_compile_options(-finput-charset=utf-8 -fexec-charset=utf-8)
    endif()
    add_compile_options(-Wall -Wextra -Werror=return-type)
endif()

add_subdirectory(imgui_mk)
add_subdirectory(imgui)
add_subdirectory(glad)

# apps/（如 passman）为可选应用层：目录就位时才接入（passman_core 仅 Release /MD 树存在）
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/apps/CMakeLists.txt")
    add_subdirectory(apps)
endif()


add_subdirectory(test)

# ============ 安装到 C:\Cpp_Libs 的静态库包（find_package(mkui) 可发现）============
# 部署布局：
#   include/imgui_mk/...  include/Mainwindow.h ...   （三个目标的头文件树）
#   include/imgui/...  include/glad/...
#   lib/mkui.lib  lib/imgui.lib  lib/glad.lib
#   lib/cmake/mkui/mkuiConfig.cmake + mkuiTargets.cmake（导出 Mk::mkui / Mk::imgui / Mk::glad）
# 消费方：set(CMAKE_PREFIX_PATH "C:/Cpp_Libs/mkui") 后 find_package(mkui REQUIRED)；
# glfw3 与 Assimp 由消费方环境提供（与仓库自身一致）
include(CMakePackageConfigHelpers)
set(mkui_VERSION 0.1.0)

install(TARGETS imgui_mk imgui glad
    EXPORT mkuiTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

install(DIRECTORY imgui_mk/include/ DESTINATION include)
install(DIRECTORY imgui/include/    DESTINATION include)
install(DIRECTORY glad/include/     DESTINATION include)

install(EXPORT mkuiTargets
    FILE mkuiTargets.cmake
    NAMESPACE Mk::
    DESTINATION lib/cmake/mkui)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/mkuiConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/mkuiConfig.cmake"
    INSTALL_DESTINATION lib/cmake/mkui)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/mkuiConfigVersion.cmake"
    VERSION ${mkui_VERSION}
    COMPATIBILITY SameMajorVersion)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/mkuiConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/mkuiConfigVersion.cmake"
    DESTINATION lib/cmake/mkui)
