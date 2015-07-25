PROJECT(neopedersia)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)

#set(CMAKE_VERBOSE_MAKEFILE on)

IF(NOT EXISTS ${PROJECT_SOURCE_DIR}/extlibs/libSDL2pp/CMakeLists.txt)
	MESSAGE(FATAL_ERROR "The source directory\n  ${PROJECT_SOURCE_DIR}/extlibs/libSDL2pp\ndoes not contain a CMakeLists.txt file.\nIt is likely that you forgot to run\n  git submodule init && git submodule update")
ENDIF(NOT EXISTS ${PROJECT_SOURCE_DIR}/extlibs/libSDL2pp/CMakeLists.txt)

IF(MSVC)
	SET(CMAKE_PREFIX_PATH
		"C:/usr/include"
		"C:/usr/lib"
		)
ELSE(MSVC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic")
ENDIF(MSVC)

IF(MINGW)
	SET(CMAKE_PREFIX_PATH
		"C:/MinGW/include"
		"C:/MinGW/lib"
		)
ENDIF(MINGW)

SET(SDL2PP_WITH_IMAGE TRUE)
SET(SDL2PP_WITH_TTF TRUE)
ADD_SUBDIRECTORY(extlibs/libSDL2pp)

INCLUDE_DIRECTORIES(SYSTEM ${SDL2PP_INCLUDE_DIRS})

ADD_SUBDIRECTORY(src)
