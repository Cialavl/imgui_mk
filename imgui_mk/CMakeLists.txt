file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)
add_library(imgui_mk STATIC ${srcs})
target_include_directories(imgui_mk PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(imgui_mk PUBLIC imgui)
target_link_libraries(imgui_mk PUBLIC glad)
