cmake_minimum_required(VERSION 3.0)
project(fontbm)

# freetype
find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})

# GCC/clang flags
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic")
endif(NOT MSVC)

set(SOURCES
        src/main.cpp
        src/App.cpp
        src/App.h
        src/FontInfo.cpp
        src/FontInfo.h
        src/ProgramOptions.cpp
        src/ProgramOptions.h
        src/GlyphInfo.h
        src/cxxopts.hpp
        src/Config.h
        src/json.hpp
        src/HelpException.h
        src/utils/extractFileName.h
        src/utils/splitStrByDelim.h
        src/utils/getNumberLen.h
        src/tinyxml2/tinyxml2.cpp
        src/tinyxml2/tinyxml2.h
        src/maxRectsBinPack/MaxRectsBinPack.cpp
        src/maxRectsBinPack/MaxRectsBinPack.h
        src/maxRectsBinPack/Rect.h
        src/freeType/FtLibrary.h
        src/freeType/FtFont.h
        src/freeType/FtException.h
        src/freeType/FtInclude.h
        src/freeType/FtLibrary.cpp
        src/lodepng/lodepng.h
        src/lodepng/lodepng.cpp
        )

add_definitions(-DLODEPNG_NO_COMPILE_DECODER)

add_executable(fontbm ${SOURCES})
target_link_libraries(fontbm ${COMMON_LIBRARIES} ${FREETYPE_LIBRARIES})

add_executable(tests
        src/catch.hpp
        src/catchMain.cpp
        src/ProgramOptions.cpp
        src/utils/getNumberLenTest.cpp
        src/utils/splitStrByDelimTest.cpp
        src/utils/extractFileNameTest.cpp
        src/ProgramOptionsTest.cpp)
target_link_libraries(tests ${COMMON_LIBRARIES})
