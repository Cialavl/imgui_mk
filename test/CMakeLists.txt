# wg2 测试整合：gtest 单元测试（一个文件一个验证点，内含多个用例）

# —— 单元测试：gtest（FetchContent 源码构建，随各配置自动匹配 CRT/ASan 标志）——
# 测试源码统一命名 *_test.cpp，全部编入 unit_tests 一个可执行文件
include(CTest)
if(BUILD_TESTING)
    include(FetchContent)
    set(INSTALL_GTEST OFF CACHE BOOL "gtest install off (mkui package)" FORCE)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/releases/download/v1.16.0/googletest-1.16.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(googletest)

    # 测试分层（浅 → 深）：基础类型 / 单组件 / 布局组合 / 消息循环 / 组件间通信
    # 一个文件一个验证点（可含多个用例）：文件名即验证主题
    set(unit_srcs
        # 第 0 层：基础类型与静态路径
        wg2_core_test.cpp
        # 第一层：单个组件
        wg2_components_basic_test.cpp
        wg2_components_input_test.cpp
        wg2_components_display_test.cpp
        wg2_components_color_test.cpp
        wg2_components_decorator_test.cpp
        wg2_components_menu_popup_test.cpp
        wg2_components_table_test.cpp
        wg2_components_text_ellipsis_test.cpp
        # 第二层：布局组合
        wg2_layout_vbox_test.cpp
        wg2_layout_hbox_test.cpp
        wg2_layout_grid_wrap_test.cpp
        wg2_layout_nested_test.cpp
        # 第三层：消息循环
        wg2_loop_input_test.cpp
        wg2_loop_frame_test.cpp
        # 第四层：组件间通信
        wg2_comm_binding_test.cpp
        wg2_comm_dynamic_test.cpp
        wg2_comm_engine_test.cpp
        # 窗口层
        wg2_window_ui_test.cpp
        # 其它模块
        mk_string_test.cpp)

    add_executable(unit_tests ${unit_srcs})
    target_link_libraries(unit_tests PRIVATE imgui_mk GTest::gtest_main)
    add_test(NAME unit_tests COMMAND unit_tests)
    # 按层拆开跑：ctest -R Wg2Layout 只跑布局层
    foreach(suite Wg2Core Wg2Component Wg2Layout Wg2Loop Wg2Comm Wg2Window)
        add_test(NAME ${suite} COMMAND unit_tests --gtest_filter=${suite}.*)
    endforeach()
endif()
