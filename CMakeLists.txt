cmake_minimum_required(VERSION 3.29)
project(RTGP-Project)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)
include_directories(src)

find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${RTGP-Project_SOURCE_DIR})
add_executable(RTGP-Project main.cpp "include/glad/glad.c")

target_link_libraries(RTGP-Project -lglfw)
target_link_libraries(RTGP-Project -lassimp)
