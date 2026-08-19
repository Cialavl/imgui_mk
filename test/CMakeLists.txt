# wg-only 测试整合：GUI 示例（引擎冒烟 + wg 控件体系验证）+ gtest 单元测试
# 注意：CMake 4 启用 CTest 后保留目标名 "test"，因此 CTest 只能开在本子目录

# —— GUI 示例：引擎/窗口层（交互式或自动关窗）——
add_executable(test windowstest.cpp)
add_executable(test2 windowstest2.cpp)
add_executable(test3 windowstest3.cpp)

# —— wg 控件体系验证（自动关窗，退出码 0 = 通过）——
add_executable(test16 windowstest16.cpp)   # 容器布局/生命周期确定性断言
add_executable(test17 windowstest17.cpp)   # 全控件渲染 soak
add_executable(test18 windowstest18.cpp)   # 双顶层窗口联动
add_executable(test19 windowstest19.cpp)   # 主题切换/rescale/作用域
add_executable(test20 windowstest20.cpp)   # 分格布局（Fixed/Stretch/em）断言
add_executable(test21 windowstest21.cpp)   # 布局嵌套全量（容器/分格/子窗口互嵌 + origin 不变量）
add_executable(test22 windowstest22.cpp)   # 原子控件实测几何（真实 item 矩形 vs bounds）

foreach(tgt test test2 test3 test16 test17 test18 test19 test20 test21 test22)
    target_link_libraries(${tgt} PRIVATE imgui_mk)
endforeach()

# —— 单元测试：gtest（FetchContent 源码构建，随各配置自动匹配 CRT/ASan 标志）——
# 单测源码命名 *_test.cpp（与示例 windowstest*.cpp 区分）
include(CTest)
if(BUILD_TESTING)
    include(FetchContent)
    set(INSTALL_GTEST OFF CACHE BOOL "gtest install off (mkui package)" FORCE)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/releases/download/v1.16.0/googletest-1.16.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(googletest)
    file(GLOB unit_srcs CONFIGURE_DEPENDS *_test.cpp)
    add_executable(unit_tests ${unit_srcs})
    target_link_libraries(unit_tests PRIVATE imgui_mk GTest::gtest_main)
    add_test(NAME unit_tests COMMAND unit_tests)
endif()


