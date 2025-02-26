cmake_minimum_required(VERSION 3.29)
project(RTGP-Project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${RTGP-Project_SOURCE_DIR})
set(BUILD_BENCHMARKS ON CACHE BOOL "Build the benchmarks executable")

include_directories(imgui)
include_directories(imgui/backends)
include_directories(imguizmo/imGuIZMO.quat)
include_directories(include)
include_directories(src)

find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)
find_package(glm REQUIRED)

add_executable(RTGP-Project
        main.cpp
        "include/glad/glad.c"
        "imgui/backends/imgui_impl_glfw.cpp"
        "imgui/backends/imgui_impl_opengl3.cpp"
        "imgui/imgui.cpp"
        "imgui/imgui_demo.cpp"
        "imgui/imgui_draw.cpp"
        "imgui/imgui_tables.cpp"
        "imgui/imgui_widgets.cpp"
        "imguizmo/imGuIZMO.quat/imGuIZMOquat.cpp"
)

if(BUILD_BENCHMARKS)
    include(FindPkgConfig)
    if(NOT PKG_CONFIG_FOUND)
        message(FATAL_ERROR "pkg-config not found!")
    endif()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(benchmark REQUIRED IMPORTED_TARGET GLOBAL benchmark)
    add_executable(Benchmarks-RTGP-Project
            benchmarks.cpp
            "include/glad/glad.c"
    )
endif()

if (WIN32)
    message("Windows specific configuration")
    target_link_libraries(RTGP-Project PRIVATE glfw)
    target_link_libraries(RTGP-Project PRIVATE assimp::assimp)
    target_link_libraries(RTGP-Project PRIVATE glm::glm-header-only)
    if(BUILD_BENCHMARKS)
        target_link_libraries(Benchmarks-RTGP-Project PRIVATE glfw)
        target_link_libraries(Benchmarks-RTGP-Project PRIVATE assimp::assimp)
        target_link_libraries(Benchmarks-RTGP-Project PRIVATE glm::glm-header-only)
        target_link_libraries(Benchmarks-RTGP-Project PRIVATE PkgConfig::benchmark)
    endif()
elseif (UNIX)
    message("Linux specific configuration")
    target_link_libraries(RTGP-Project -lglfw)
    target_link_libraries(RTGP-Project -lassimp)
    if(BUILD_BENCHMARKS)
        target_link_libraries(Benchmarks-RTGP-Project -lglfw)
        target_link_libraries(Benchmarks-RTGP-Project -lassimp)
        target_link_libraries(Benchmarks-RTGP-Project -lbenchmark)
    endif()
endif ()