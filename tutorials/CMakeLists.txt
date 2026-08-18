# mkui 教学课程配套程序：tutorial00..12，每课一个可运行的活教材。
# 无外部依赖，Debug/ASan/Release 全树构建；交互式（手动关窗退出），
# 例外：带自演示计时的课程按各自注释说明。
file(GLOB tutorial_srcs CONFIGURE_DEPENDS src/*.cpp)
foreach(src ${tutorial_srcs})
    get_filename_component(name ${src} NAME_WE)
    add_executable(${name} ${src})
    target_link_libraries(${name} PRIVATE imgui_mk)
endforeach()