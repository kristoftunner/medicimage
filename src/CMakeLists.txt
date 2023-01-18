cmake_minimum_required(VERSION 3.16)

file(GLOB medicimage_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
message(STATUS "Collected MEDICIMAGE source: ${medicimage_sources}")
add_library(medicimage STATIC ${medicimage_sources})
set_property(TARGET medicimage PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
#set_property(TARGET medicimage PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:>")

target_link_libraries(medicimage PUBLIC ${OpenCV_LIBS} SDL2::SDL2 SDL2::SDL2main imgui d3d11.lib dxgi.lib d3dcompiler.lib dxguid.lib stb_image spdlog::spdlog)
target_include_directories(medicimage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS} ${IMGUI_DIR})

if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW) 
endif()