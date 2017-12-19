cmake_minimum_required(VERSION 2.8)
project(fontbm)

# freetype
find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})

# additional libraries for windows platform (winmm.lib and sdl2main.lib)
if(WIN32)
    find_library(SDL2MAIN_LIBRARY NAMES SDL2main)
    set(SDL2PP_LIBRARIES ${SDL2MAIN_LIBRARY} ${SDL2PP_LIBRARIES})
    set(COMMON_LIBRARIES winmm.lib ${COMMON_LIBRARIES})
endif(WIN32)

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
