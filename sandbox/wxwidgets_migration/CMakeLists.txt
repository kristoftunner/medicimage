find_package(wxWidgets CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(MacroShot WIN32 main.cpp )
target_link_libraries(MacroShot PRIVATE OpenGL::GL medicimage)
set_property(TARGET MacroShot PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
