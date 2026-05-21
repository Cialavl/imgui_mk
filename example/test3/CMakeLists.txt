file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)
add_executable(test3 ${srcs})
target_include_directories(test3 PUBLIC include)
target_link_libraries(test3 PUBLIC imgui_mk)