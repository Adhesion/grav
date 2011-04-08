# @file FindFTGL.cmake
#
# Find FTGL libs and include paths.
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

