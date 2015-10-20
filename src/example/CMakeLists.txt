set(SOURCES
	Example.cpp
	BmFontPrinter.cpp
	BmFont.cpp
)

add_executable(example ${SOURCES})
target_link_libraries(example ${COMMON_LIBRARIES} ${SDL2PP_LIBRARIES})
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
