file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp src/*.c include/*.h)
add_library(imgui STATIC ${srcs})
find_package(glfw3 REQUIRED)
target_link_libraries(imgui PUBLIC glfw)

find_package(Assimp REQUIRED)
target_link_libraries(imgui PUBLIC assimp::assimp)



target_include_directories(imgui PUBLIC include)
