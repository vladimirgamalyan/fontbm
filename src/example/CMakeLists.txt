set(SOURCES
	Example.cpp
	BmFont.cpp
    FntInfo.cpp
)

add_executable(example ${SOURCES})
target_link_libraries(example ${COMMON_LIBRARIES} ${SDL2PP_LIBRARIES})
