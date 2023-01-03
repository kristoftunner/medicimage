cmake_minimum_required(VERSION 3.16)

add_executable(medicimage sandbox.cpp imgui_build.cpp renderer.cpp)
set_property(TARGET medicimage PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
target_link_libraries(medicimage PUBLIC ${OpenCV_LIBS} SDL2::SDL2 SDL2::SDL2main imgui d3d11.lib dxgi.lib d3dcompiler.lib dxguid.lib)
target_include_directories(medicimage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS} ${IMGUI_DIR})