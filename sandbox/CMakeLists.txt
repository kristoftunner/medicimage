cmake_minimum_required(VERSION 3.16)

find_package(wxWidgets CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

if(False)
  add_executable(medical_imaging sandbox.cpp)
  target_link_libraries(medical_imaging PUBLIC medicimage)
  set_property(TARGET medical_imaging PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  install(DIRECTORY assets DESTINATION "")

  add_executable(ocv opencv_tries.cpp)
  target_link_libraries(ocv PUBLIC ${OpenCV_LIBS} medicimage)
  set_property(TARGET ocv PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  add_executable(sandbox_old sandbox_old.cpp)
  target_link_libraries(sandbox_old PUBLIC medicimage)
  set_property(TARGET sandbox_old PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_subdirectory(wxwidgets_migration)

add_executable(layout WIN32 layout/example.cpp layout/example.h)
target_link_libraries(layout PUBLIC wx::core wx::base wx::wxgl OpenGL::GL medicimage)
target_include_directories(layout PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_property(TARGET layout PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(keyboard WIN32 keyboard/keyboard.cpp)
target_link_libraries(keyboard PUBLIC wx::core wx::base wx::wxgl OpenGL::GL medicimage)
set_property(TARGET keyboard PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

