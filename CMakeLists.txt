cmake_minimum_required(VERSION 3.11)
project(fontbm)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})

find_package(HarfBuzz REQUIRED)
include_directories(${HARFBUZZ_INCLUDE_DIR})

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
        src/external/cxxopts.hpp
        src/Config.h
        src/external/json.hpp
        src/HelpException.h
        src/utils/extractFileName.h
        src/utils/splitStrByDelim.h
        src/utils/splitStrByDelim.cpp
        src/utils/StringMaker.h
        src/utils/getNumberLen.h
        src/freeType/FtLibrary.h
        src/freeType/FtFont.h
        src/freeType/FtException.h
        src/freeType/FtInclude.h
        src/freeType/FtLibrary.cpp
        src/external/utf8cpp/utf8.h
        src/external/utf8cpp/utf8/core.h
        src/external/utf8cpp/utf8/unchecked.h
        src/external/utf8cpp/utf8/checked.h
        src/external/tinyxml2/tinyxml2.cpp
        src/external/tinyxml2/tinyxml2.h
        src/external/maxRectsBinPack/MaxRectsBinPack.cpp
        src/external/maxRectsBinPack/MaxRectsBinPack.h
        src/external/maxRectsBinPack/Rect.h
        src/external/lodepng/lodepng.h
        src/external/lodepng/lodepng.cpp
        )

add_definitions(-DLODEPNG_NO_COMPILE_DECODER)

add_executable(fontbm ${SOURCES})
target_link_libraries(fontbm ${COMMON_LIBRARIES} ${FREETYPE_LIBRARIES} harfbuzz::harfbuzz)

add_executable(unit_tests
        src/external/catch.hpp
        src/catchMain.cpp
        src/ProgramOptions.cpp
        src/utils/splitStrByDelim.cpp
        src/utils/getNumberLenTest.cpp
        src/utils/splitStrByDelimTest.cpp
        src/utils/extractFileNameTest.cpp
        src/utils/StringMaker.h
        src/ProgramOptionsTest.cpp
        )
target_link_libraries(unit_tests ${COMMON_LIBRARIES})
