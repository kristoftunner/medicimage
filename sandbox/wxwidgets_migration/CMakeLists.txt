find_package(wxWidgets CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(app WIN32 main.cpp )
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE includes ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
target_sources(app PUBLIC ${sources} ${includes})

target_link_libraries(app PUBLIC wx::core wx::base wx::wxgl OpenGL::GL medicimage)
set_property(TARGET app PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
