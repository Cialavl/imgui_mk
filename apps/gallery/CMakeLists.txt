# mkui 控件画廊应用：双库（OOP + 函数式）全量交互展示，输出 gallery.exe
# 无外部依赖，Debug/ASan/Release 全树可构建
file(GLOB gallery_srcs CONFIGURE_DEPENDS src/*.cpp)
add_executable(gallery ${gallery_srcs})
target_include_directories(gallery PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(gallery PRIVATE imgui_mk)