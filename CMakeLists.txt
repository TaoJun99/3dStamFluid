cmake_minimum_required(VERSION 3.26)
project(3dStamFluid)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCE_FILES  main.cpp shader.frag shader.vert)

add_executable(3dStamFluid main.cpp)

find_package(glfw3 REQUIRED)
target_link_libraries(3dStamFluid PRIVATE glfw)

find_package(GLEW REQUIRED)
target_link_libraries(3dStamFluid PRIVATE GLEW::GLEW)

find_package(glm REQUIRED)
target_include_directories(3dStamFluid PRIVATE ${GLM_INCLUDE_DIRS})

# Link OpenGL
target_link_libraries(3dStamFluid PRIVATE ${OPENGL_gl_LIBRARY})
