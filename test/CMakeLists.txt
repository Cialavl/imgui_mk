# 统一测试整合：GUI 示例（test / test2 / test3，兼作 GUI 冒烟测试目标）+ gtest 单元测试
# 注意：CMake 4 启用 CTest 后保留目标名 "test"，因此 CTest 只能开在本子目录

# GUI 示例目标（源码平铺在本目录）
add_executable(test windowstest.cpp)
target_link_libraries(test PRIVATE imgui_mk)

add_executable(test2 windowstest2.cpp)
target_link_libraries(test2 PRIVATE imgui_mk)

add_executable(test3 windowstest3.cpp)
target_link_libraries(test3 PRIVATE imgui_mk)

# 控件树验证示例：自动渲染约 90 帧后关闭，退出码 0 表示通过
add_executable(test4 windowstest4.cpp)
target_link_libraries(test4 PRIVATE imgui_mk)

# 控件树示例：演示 setWidget 挂载、布局容器、动态增删子控件（交互式，手动关窗退出）
add_executable(test5 windowstest5.cpp)
target_link_libraries(test5 PRIVATE imgui_mk)

# 嵌套多窗口示例：双顶层窗口 + 三层嵌套子窗口 + 跨窗口数据联动（交互式，全关退出）
add_executable(test6 windowstest6.cpp)
target_link_libraries(test6 PRIVATE imgui_mk)

# 单元测试：gtest（FetchContent 源码构建，随各配置自动匹配 CRT/ASan 标志）
# 单测源码命名 *_test.cpp（与示例 windowstest*.cpp 区分）
include(CTest)
if(BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/releases/download/v1.16.0/googletest-1.16.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(googletest)
    file(GLOB unit_srcs CONFIGURE_DEPENDS *_test.cpp)
    add_executable(unit_tests ${unit_srcs})
    target_link_libraries(unit_tests PRIVATE imgui_mk GTest::gtest_main)
    add_test(NAME unit_tests COMMAND unit_tests)
endif()