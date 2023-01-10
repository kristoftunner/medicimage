cmake_minimum_required(VERSION 3.16)

file(GLOB medicimage_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
message(STATUS "Collected MEDICIMAGE source: ${medicimage_sources}")
add_executable(medicimage ${medicimage_sources})
set_property(TARGET medicimage PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
target_link_libraries(medicimage PUBLIC ${OpenCV_LIBS} SDL2::SDL2 SDL2::SDL2main imgui d3d11.lib dxgi.lib d3dcompiler.lib dxguid.lib stb_image)
target_include_directories(medicimage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS} ${IMGUI_DIR})

#add_executable(sandbox sandbox_old.cpp imgui_build.cpp )
#set_property(TARGET sandbox PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
#target_link_libraries(sandbox PUBLIC ${OpenCV_LIBS} SDL2::SDL2 SDL2::SDL2main imgui d3d11.lib dxgi.lib d3dcompiler.lib dxguid.lib)
#target_include_directories(sandbox PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS} ${IMGUI_DIR})
