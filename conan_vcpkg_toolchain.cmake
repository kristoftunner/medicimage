include("d:\\dev\\repos\\imguiapp\\build\\conan_toolchain.cmake")

set(VCPKG_TARGET_TRIPLET "x64-windows-static") # Replace with your target triplet
set(CMAKE_TOOLCHAIN_FILE "d:\\dev\\libs\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")
