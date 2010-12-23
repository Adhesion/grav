# @file FindGLEW.cmake
#
# Find GLEW libs and include paths.
#
# Created on: Dec 22, 2010
# @author Andrew Ford

include(FindPkgConfig)

if(PKG_CONFIG_FOUND)
	message("Doing pkg config glew check...")
	pkg_check_modules(GLEW
		glew>=1.5
		)
endif()

if(NOT GLEW_FOUND)
	message("Warning: could not confirm GLEW version (pkg-config not available).")
	message("Ensure your GLEW version is >= 1.5 for a clean build.")

	find_path(
		GLEW_INCLUDE_DIRS
		GL/glew.h
		HINTS ${GLEW_HOME}/include
		)

	find_library(
		GLEW_LIBRARIES
		GLEW
		HINTS ${GLEW_HOME}/lib
		)

	include(FindPackageHandleStandardArgs)

	find_package_handle_standard_args(
		GLEW
		DEFAULT_MSG
		GLEW_INCLUDE_DIRS
		GLEW_LIBRARIES
		)
endif()

