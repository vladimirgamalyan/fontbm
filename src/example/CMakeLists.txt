set(SOURCES
	Example.cpp
	BmFontPrinter.cpp
	BmFont.cpp
	FntFileReader.cpp
)

add_executable(example ${SOURCES})
target_link_libraries(example ${COMMON_LIBRARIES} ${SDL2PP_LIBRARIES})
