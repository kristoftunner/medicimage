find_package(wxWidgets CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(app WIN32 main.cpp )
target_link_libraries(app PRIVATE OpenGL::GL medicimage)
set_property(TARGET app PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
