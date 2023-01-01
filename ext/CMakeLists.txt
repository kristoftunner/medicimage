cmake_minimum_required(VERSION 3.16)

add_subdirectory(imgui)
add_subdirectory(spdlog)

# setup cmake options
set(BUILD_opencv_python_bindings_generator "" OFF)
set(BUILD_opencv_python_tests "" OFF)
set(BUILD_opencv_python3 "" OFF)
set(BUILD_SHARED_LIBS "" OFF)
set(BUILD_EXAMPLES "" OFF)
set(BUILD_opencv_apps "" OFF)
add_subdirectory(opencv)