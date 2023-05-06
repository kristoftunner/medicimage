find_package(wxWidgets CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(app WIN32 main.cpp canvas.cpp)
target_link_libraries(app PUBLIC wx::core wx::base wx::wxgl OpenGL::GL medicimage)
target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_property(TARGET app PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
