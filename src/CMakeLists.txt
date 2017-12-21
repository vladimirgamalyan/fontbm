set(SOURCES
        main.cpp
        App.cpp
        App.h
        FontInfo.cpp
        FontInfo.h
        ProgramOptions.cpp
        ProgramOptions.h
        tinyxml2/tinyxml2.cpp
        tinyxml2/tinyxml2.h
        GlyphInfo.h
        cxxopts.hpp
        Config.h
        json.hpp
        HelpException.h
        extractFileName.h
        splitStrByDelim.h
        maxRectsBinPack/MaxRectsBinPack.cpp
        maxRectsBinPack/MaxRectsBinPack.h
        maxRectsBinPack/Rect.h
        utfcpp/utf8.h
        utfcpp/utf8/core.h
        utfcpp/utf8/unchecked.h
        utfcpp/utf8/checked.h
		freeType/FtLibrary.h
		freeType/FtFace.h
		freeType/FtException.h
		freeType/FtInclude.h
		freeType/FtLibrary.cpp
		lodepng/lodepng.h
		lodepng/lodepng.cpp
        )

add_definitions(-DLODEPNG_NO_COMPILE_DECODER)

add_executable(fontbm ${SOURCES})
target_link_libraries(fontbm ${COMMON_LIBRARIES} ${FREETYPE_LIBRARIES})
