# wg2 测试整合：引擎冒烟 + wg2 控件体系验证 + gtest 单元测试
# 注意：CMake 4 启用 CTest 后保留目标名 "test"，因此 CTest 只能开在本子目录

# —— GUI 示例：引擎/窗口层（交互式或自动关窗）——
add_executable(test windowstest.cpp)
add_executable(test2 windowstest2.cpp)
add_executable(test3 windowstest3.cpp)

# —— wg2 控件体系验证 ——
add_executable(wg2_smoke wg2_smoke.cpp)    # wg2 静态控件体系离线冒烟

foreach(tgt test test2 test3 wg2_smoke)
    target_link_libraries(${tgt} PRIVATE imgui_mk)
endforeach()

if(BUILD_TESTING)
    add_test(NAME wg2_smoke COMMAND wg2_smoke)
endif()

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
    set(unit_srcs wg2_test.cpp mk_string_test.cpp mkui_test.cpp)
    add_executable(unit_tests ${unit_srcs})
    target_link_libraries(unit_tests PRIVATE imgui_mk GTest::gtest_main)
    add_test(NAME unit_tests COMMAND unit_tests)
endif()


