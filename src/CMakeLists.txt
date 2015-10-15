set(SOURCES
	main.cpp
	Font.cpp
	maxRectsBinPack/MaxRectsBinPack.cpp
	sdlSavePng/savepng.c
	tinyxml2/tinyxml2.cpp
    ProgramOptions.cpp
)

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs")
endif(CMAKE_COMPILER_IS_GNUCC)

if(MINGW)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    SET(MINGW32_LIBRARY mingw32 CACHE STRING "mwindows for MinGW")
    FIND_LIBRARY(SDL2MAIN_LIBRARY NAMES SDL2main)
    SET(SDL2PP_LIBRARIES ${MINGW32_LIBRARY} ${SDL2MAIN_LIBRARY} ${SDL2PP_LIBRARIES})
endif(MINGW)

if(MSVC)
	FIND_LIBRARY(SDL2MAIN_LIBRARY NAMES SDL2main)
	SET(SDL2PP_LIBRARIES winmm.lib ${SDL2MAIN_LIBRARY} ${SDL2PP_LIBRARIES})
endif(MSVC)

add_executable(fontbm ${SOURCES})
target_link_libraries(fontbm ${PNG_LIBRARIES} ${FREETYPE_LIBRARIES} ${SDL2PP_LIBRARIES} ${Boost_LIBRARIES})
