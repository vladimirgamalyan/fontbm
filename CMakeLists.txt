cmake_minimum_required(VERSION 3.0)
project(fontbm)

# freetype
find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})

# GCC/clang flags
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic")
endif(NOT MSVC)

add_subdirectory(src)
add_subdirectory(src/tests)
