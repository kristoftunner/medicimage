cmake_minimum_required(VERSION 3.16)

project("medicimage" CXX)

# setup cmake globally
include(cmake/setup_env.cmake)

# add the dependencies
#add_subdirectory(ext)
find_package(OpenCV REQUIRED PATHS "d:\\dev\\repos\\opencv\\build\\install\\x64\\vc17\\staticlib") # first build openCV in ext then get the static libs from there
message(STATUS "opencv libs: ${OpenCV_LIBS} includes:${OpenCV_INCLUDE_DIRS}")
# this workaround is needed because of this OpenCV build issue: https://github.com/opencv/opencv/issues/22348
foreach(opencv_package ${OpenCV_LIBS})
  set_target_properties(${opencv_package} PROPERTIES
    MAP_IMPORTED_CONFIG_MINSIZEREL Debug
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO Debug 
  )
endforeach()
add_subdirectory(ext/sdl)
add_subdirectory(ext/imgui)
add_subdirectory(ext/stb)
add_subdirectory(ext/spdlog)
add_subdirectory(ext/json)
add_subdirectory(ext/entt)
#set_property(TARGET spdlog PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:>")
set_property(TARGET spdlog PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
# the actual library build
add_subdirectory(src)

# sandbox and test applications
add_subdirectory(sandbox)