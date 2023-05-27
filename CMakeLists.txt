cmake_minimum_required(VERSION 3.16)
set(CMAKE_TOOLCHAIN_FILE "d:/dev/libs/vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_TARGET_TRIPLET "x64-windows-static")
project("medicimage" CXX)
# setup cmake globally
include(cmake/setup_env.cmake)

# add the dependencies
add_subdirectory(ext/stb)
add_subdirectory(ext/spdlog)
add_subdirectory(ext/json)

set_property(TARGET spdlog PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
# the actual library build
add_subdirectory(src)

# sandbox and test applications
add_subdirectory(sandbox)
