set(SOURCES
	    main.cpp
	    FontInfo.cpp
	    maxRectsBinPack/MaxRectsBinPack.cpp
	    sdlSavePng/savepng.c
        tinyxml2/tinyxml2.cpp
        ProgramOptions.cpp
        App.cpp
        App.h
        GlyphInfo.h
        cxxopts.hpp
        HelpException.h)

add_executable(fontbm ${SOURCES})
target_link_libraries(fontbm ${COMMON_LIBRARIES} ${PNG_LIBRARIES} ${FREETYPE_LIBRARIES} ${SDL2PP_LIBRARIES} ${Boost_LIBRARIES})
