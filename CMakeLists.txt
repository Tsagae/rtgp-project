cmake_minimum_required(VERSION 3.29)
project(RTGP-Project)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)
include_directories(src)
include_directories(${RTGP-Project_SOURCE_DIR}/vcpkg/installed/x64-windows/include)


find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)
find_package(glm REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${RTGP-Project_SOURCE_DIR})
add_executable(RTGP-Project main.cpp "include/glad/glad.c")

set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
target_compile_options(RTGP-Project PUBLIC /MT)
target_link_options(RTGP-Project PUBLIC /INCREMENTAL:NO /NODEFAULTLIB:MSVCRT)

link_libraries("-static")

target_link_libraries(RTGP-Project PRIVATE glfw -static)
target_link_libraries(RTGP-Project PRIVATE assimp::assimp -static)
target_link_libraries(RTGP-Project PRIVATE glm::glm -static)