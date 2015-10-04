set(SOURCES
	main.cpp
	Font.cpp
	maxRectsBinPack/MaxRectsBinPack.cpp
	sdlSavePng/savepng.c
	tinyxml2/tinyxml2.cpp
	ConfigFile.cpp
	CharList.cpp
)

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
target_link_libraries(fontbm ${PNG_LIBRARIES} ${SDL2PP_LIBRARIES} ${Boost_LIBRARIES})
