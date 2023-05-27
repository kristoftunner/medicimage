cmake_minimum_required(VERSION 3.16)

find_package(glm REQUIRED)
find_package(entt REQUIRED)
find_package(wxWidgets CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

file(GLOB_RECURSE medicimage_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
message(STATUS "Collected MEDICIMAGE source: ${medicimage_sources}")
add_library(medicimage STATIC ${medicimage_sources})
set_property(TARGET medicimage PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(OpenCV_DIR "${VCPKG_INSTALLED_DIR}/x64-windows-static/share/opencv4")
find_package(OpenCV REQUIRED)

target_link_libraries(medicimage PUBLIC ${OpenCV_LIBS} stb_image 
  EnTT::EnTT spdlog::spdlog glm::glm wx::core wx::base wx::wxgl)
target_include_directories(medicimage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS} ${JSON_INCLUDE_DIR} ${glm_INCLUDE_DIRS_DEBUG})
target_compile_definitions(medicimage PUBLIC NOMINMAX)
set_property(TARGET spdlog PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW) 
endif()
