# @file FindGLEW.cmake
#
# Find GLEW libs and include paths.
#
# Created on: Dec 22, 2010
# @author Andrew Ford
# Copyright (C) 2011 Rochester Institute of Technology
#
# This file is part of grav.
#
# grav is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# grav is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with grav.  If not, see <http://www.gnu.org/licenses/>.

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

