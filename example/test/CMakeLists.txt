file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)
add_executable(test ${srcs})
target_include_directories(test PUBLIC include)
target_link_libraries(test PUBLIC imgui_mk)