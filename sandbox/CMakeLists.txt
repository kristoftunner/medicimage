cmake_minimum_required(VERSION 3.16)

add_executable(sandbox sandbox.cpp)
target_link_libraries(sandbox PUBLIC medicimage)
set_property(TARGET sandbox PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(sandbox_old sandbox_old.cpp)
target_link_libraries(sandbox_old PUBLIC medicimage)
set_property(TARGET sandbox_old PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")