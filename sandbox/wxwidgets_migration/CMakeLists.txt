find_package(wxWidgets CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(medical_imaging WIN32 main.cpp )
target_link_libraries(medical_imaging PRIVATE OpenGL::GL medicimage)
set_property(TARGET medical_imaging PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
