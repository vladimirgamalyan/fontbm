cmake_minimum_required(VERSION 3.0)
project(fontbm)

# freetype
find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})

# additional libraries for mingw32 (mingw32.lib)
if(MINGW)
    set(MINGW32_LIBRARY mingw32 CACHE STRING "mwindows for MinGW")
    set(COMMON_LIBRARIES mingw32 ${COMMON_LIBRARIES})
endif(MINGW)

# GCC/clang flags
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic")
endif(NOT MSVC)

# disable some GCC warnings
if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-unused-local-typedefs")
endif(CMAKE_COMPILER_IS_GNUCC)

add_subdirectory(src)
add_subdirectory(src/tests)
