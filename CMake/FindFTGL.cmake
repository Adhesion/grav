# @file FindFTGL.cmake
#
# Find FTGL libs and include paths.
#
# Created on: Dec 22, 2010
# @author Andrew Ford

find_package(Freetype)

find_path(
	FTGL_INCLUDE_DIRS
	FTGL/ftgl.h
	HINTS ${FTGL_HOME}/include
	)

find_library(
	FTGL_LIBRARIES
	ftgl
	HINTS ${FTGL_HOME}/lib
	)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
	FTGL
	DEFAULT_MSG
	FTGL_INCLUDE_DIRS
	FTGL_LIBRARIES
	)

