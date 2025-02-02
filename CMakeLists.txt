cmake_minimum_required(VERSION 3.29)
project(RTGP-Project)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)
include_directories(src)

find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)
find_package(glm REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${RTGP-Project_SOURCE_DIR})
add_executable(RTGP-Project main.cpp "include/glad/glad.c")

if(WIN32)
    message("Windows specific configuration")
    target_link_libraries(RTGP-Project PRIVATE glfw)
    target_link_libraries(RTGP-Project PRIVATE assimp::assimp)
    target_link_libraries(RTGP-Project PRIVATE glm::glm)
elseif(UNIX)
    message("Linux specific configuration")
    target_link_libraries(RTGP-Project -lglfw)
    target_link_libraries(RTGP-Project -lassimp)
endif()