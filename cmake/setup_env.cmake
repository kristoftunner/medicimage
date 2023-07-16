cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(BUILD_SHARED_LIBS OFF)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")   
add_compile_options("/W4")