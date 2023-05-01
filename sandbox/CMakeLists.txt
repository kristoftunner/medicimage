cmake_minimum_required(VERSION 3.16)

add_executable(medical_imaging sandbox.cpp)
target_link_libraries(medical_imaging PUBLIC medicimage)
set_property(TARGET medical_imaging PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
install(DIRECTORY assets DESTINATION "")

if(False)
  add_executable(ocv opencv_tries.cpp)
  target_link_libraries(ocv PUBLIC ${OpenCV_LIBS} medicimage)
  set_property(TARGET ocv PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
add_executable(sandbox_old sandbox_old.cpp)
target_link_libraries(sandbox_old PUBLIC medicimage)
set_property(TARGET sandbox_old PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
