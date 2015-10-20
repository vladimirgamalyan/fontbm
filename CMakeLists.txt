cmake_minimum_required(VERSION 2.8)
project(fontbm)

# boost
set(BOOST_MIN_VERSION "1.46.0")
find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS system filesystem program_options regex)
include_directories(${Boost_INCLUDE_DIR})

# libpng
find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIRS})

# freetype
find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})

# libsdl2pp
if(NOT EXISTS ${PROJECT_SOURCE_DIR}/extlibs/libSDL2pp/CMakeLists.txt)
    message(FATAL_ERROR "The source directory\n  ${PROJECT_SOURCE_DIR}/extlibs/libSDL2pp\ndoes not contain a CMakeLists.txt file.\nIt is likely that you forgot to run\n  git submodule init && git submodule update")
endif(NOT EXISTS ${PROJECT_SOURCE_DIR}/extlibs/libSDL2pp/CMakeLists.txt)
set(SDL2PP_WITH_TTF TRUE)
add_subdirectory(extlibs/libSDL2pp)
include_directories(SYSTEM ${SDL2PP_INCLUDE_DIRS})

# additional libraries for windows platform (winmm.lib and sdl2main.lib)
if(WIN32)
    FIND_LIBRARY(SDL2MAIN_LIBRARY NAMES SDL2main)
    SET(SDL2PP_LIBRARIES ${SDL2MAIN_LIBRARY} ${SDL2PP_LIBRARIES})
    SET(COMMON_LIBRARIES winmm.lib ${COMMON_LIBRARIES})
endif(WIN32)

# additional mingw32 lib for mingw32
if(MINGW)
    SET(MINGW32_LIBRARY mingw32 CACHE STRING "mwindows for MinGW")
    SET(COMMON_LIBRARIES mingw32 ${COMMON_LIBRARIES})
endif(MINGW)

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-unused-local-typedefs")
endif(CMAKE_COMPILER_IS_GNUCC)

add_subdirectory(src)
add_subdirectory(src/tests)
add_subdirectory(src/example)
