cmake_minimum_required(VERSION 3.16)

project("medicimage" CXX)

# setup cmake globally
include(cmake/setup_env.cmake)

# add the dependencies
add_subdirectory(ext)