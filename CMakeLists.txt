PROJECT(fontbm)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7)

#set(CMAKE_VERBOSE_MAKEFILE on)

IF(NOT EXISTS ${PROJECT_SOURCE_DIR}/extlibs/libSDL2pp/CMakeLists.txt)
	MESSAGE(FATAL_ERROR "The source directory\n  ${PROJECT_SOURCE_DIR}/extlibs/libSDL2pp\ndoes not contain a CMakeLists.txt file.\nIt is likely that you forgot to run\n  git submodule init && git submodule update")
ENDIF(NOT EXISTS ${PROJECT_SOURCE_DIR}/extlibs/libSDL2pp/CMakeLists.txt)

IF(MINGW)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic")
	SET(CMAKE_PREFIX_PATH
		"C:/msys64/mingw32/include"
		"C:/msys64/mingw32/lib"
		)
ENDIF(MINGW)

set(BOOST_MIN_VERSION "1.46.0")

FIND_PACKAGE(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS system filesystem program_options regex)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

FIND_PACKAGE(PNG REQUIRED)
INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIRS})

SET(SDL2PP_WITH_IMAGE TRUE)
SET(SDL2PP_WITH_TTF TRUE)
ADD_SUBDIRECTORY(extlibs/libSDL2pp)

INCLUDE_DIRECTORIES(SYSTEM ${SDL2PP_INCLUDE_DIRS})

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(src/example)
ADD_SUBDIRECTORY(src/tests)
