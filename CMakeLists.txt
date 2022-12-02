cmake_minimum_required(VERSION 3.24)
project(ims)

set(CMAKE_CXX_STANDARD 17)
include(FindSimlib.cmake)
add_executable(ims main.cpp main.h)
target_link_libraries(ims ${simlib_LIBRARY})