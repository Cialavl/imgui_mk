# 个人验证 demo：手动运行，非自动化测试。
file(GLOB dome_srcs CONFIGURE_DEPENDS src/*.cpp)
add_executable(dome ${dome_srcs})
target_link_libraries(dome PRIVATE imgui_mk)
