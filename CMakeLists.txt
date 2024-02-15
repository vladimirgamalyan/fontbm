cmake_minimum_required(VERSION 3.0)
project(fontbm)

find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})

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
        src/external/lunasvg/include/lunasvg.h
        src/external/lunasvg/source/canvas.cpp
        src/external/lunasvg/source/canvas.h
        src/external/lunasvg/source/clippathelement.cpp
        src/external/lunasvg/source/clippathelement.h
        src/external/lunasvg/source/defselement.cpp
        src/external/lunasvg/source/defselement.h
        src/external/lunasvg/source/element.cpp
        src/external/lunasvg/source/element.h
        src/external/lunasvg/source/gelement.cpp
        src/external/lunasvg/source/gelement.h
        src/external/lunasvg/source/geometryelement.cpp
        src/external/lunasvg/source/geometryelement.h
        src/external/lunasvg/source/graphicselement.cpp
        src/external/lunasvg/source/graphicselement.h
        src/external/lunasvg/source/layoutcontext.cpp
        src/external/lunasvg/source/layoutcontext.h
        src/external/lunasvg/source/lunasvg.cpp
        src/external/lunasvg/source/markerelement.cpp
        src/external/lunasvg/source/markerelement.h
        src/external/lunasvg/source/maskelement.cpp
        src/external/lunasvg/source/maskelement.h
        src/external/lunasvg/source/paintelement.cpp
        src/external/lunasvg/source/paintelement.h
        src/external/lunasvg/source/parser.cpp
        src/external/lunasvg/source/parser.h
        src/external/lunasvg/source/parserutils.h
        src/external/lunasvg/source/property.cpp
        src/external/lunasvg/source/property.h
        src/external/lunasvg/source/stopelement.cpp
        src/external/lunasvg/source/stopelement.h
        src/external/lunasvg/source/styledelement.cpp
        src/external/lunasvg/source/styledelement.h
        src/external/lunasvg/source/styleelement.cpp
        src/external/lunasvg/source/styleelement.h
        src/external/lunasvg/source/svgelement.cpp
        src/external/lunasvg/source/svgelement.h
        src/external/lunasvg/source/symbolelement.cpp
        src/external/lunasvg/source/symbolelement.h
        src/external/lunasvg/source/useelement.cpp
        src/external/lunasvg/source/useelement.h 
        src/external/lunasvg/3rdparty/plutovg/plutovg-blend.c
        src/external/lunasvg/3rdparty/plutovg/plutovg.c
        src/external/lunasvg/3rdparty/plutovg/plutovg-dash.c
        src/external/lunasvg/3rdparty/plutovg/plutovg-ft-math.c
        src/external/lunasvg/3rdparty/plutovg/plutovg-ft-math.h
        src/external/lunasvg/3rdparty/plutovg/plutovg-ft-raster.c
        src/external/lunasvg/3rdparty/plutovg/plutovg-ft-raster.h
        src/external/lunasvg/3rdparty/plutovg/plutovg-ft-stroker.c
        src/external/lunasvg/3rdparty/plutovg/plutovg-ft-stroker.h
        src/external/lunasvg/3rdparty/plutovg/plutovg-ft-types.h
        src/external/lunasvg/3rdparty/plutovg/plutovg-geometry.c
        src/external/lunasvg/3rdparty/plutovg/plutovg.h
        src/external/lunasvg/3rdparty/plutovg/plutovg-paint.c
        src/external/lunasvg/3rdparty/plutovg/plutovg-private.h
        src/external/lunasvg/3rdparty/plutovg/plutovg-rle.c
        )

add_definitions(-DLODEPNG_NO_COMPILE_DECODER)

add_executable(fontbm ${SOURCES})
target_link_libraries(fontbm ${COMMON_LIBRARIES} ${FREETYPE_LIBRARIES})

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
