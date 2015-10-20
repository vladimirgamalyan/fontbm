set(SOURCES
	main.cpp
	FontInfo.cpp
	maxRectsBinPack/MaxRectsBinPack.cpp
	sdlSavePng/savepng.c
	tinyxml2/tinyxml2.cpp
    ProgramOptions.cpp
)

add_executable(fontbm ${SOURCES})
target_link_libraries(fontbm ${COMMON_LIBRARIES} ${PNG_LIBRARIES} ${FREETYPE_LIBRARIES} ${SDL2PP_LIBRARIES} ${Boost_LIBRARIES})

enable_testing()
add_test(test fontbm --font-file "fonts/FreeSans.ttf" --output "output")
