file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)
add_executable(test2 ${srcs})
target_include_directories(test2 PUBLIC include)
target_link_libraries(test2 PUBLIC imgui_mk)