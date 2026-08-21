# mkui 教学课程配套程序：tutorial00..12，每课一个可运行的活教材。
# 无外部依赖，Debug/ASan/Release 全树构建；交互式（手动关窗退出），
# 例外：带自演示计时的课程按各自注释说明。
file(GLOB tutorial_srcs CONFIGURE_DEPENDS src/*.cpp)
# TODO-阶段2(wg Qt 化重构)：教程程序暂时禁用（新 API 迁移后恢复下面 GLOB）
# file(GLOB tutorial_srcs CONFIGURE_DEPENDS src/*.cpp)  ← 阶段2 恢复此形式，删除上面的空列表
set(tutorial_srcs "")
foreach(src ${tutorial_srcs})
    get_filename_component(name ${src} NAME_WE)
    add_executable(${name} ${src})
    target_link_libraries(${name} PRIVATE imgui_mk)
endforeach()

# wg2 新教程单独启用（旧教程 glob 仍处于禁用状态）
add_executable(tutorial13_wg2 src/tutorial13_wg2.cpp)
target_link_libraries(tutorial13_wg2 PRIVATE imgui_mk)