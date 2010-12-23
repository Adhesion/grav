# @file FindGLEW.cmake
#
# Find GLEW libs and include paths.
#
# Created on: Dec 22, 2010
# @author Andrew Ford

find_path(
	GLEW_INCLUDE_DIRS
	GL/ftgl.h
	HINTS ${GLEW_HOME}/include
	)

find_library(
	GLEW_LIBRARIES
	glew
	HINTS ${GLEW_HOME}/lib
	)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
	GLEW
	DEFAULT_MSG
	GLEW_INCLUDE_DIRS
	GLEW_LIBRARIES
	)

