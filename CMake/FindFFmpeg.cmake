# @file FindFFmpeg.cmake
#
# Find FFmpeg libs and include paths.
# Will set INCLUDE_DIRS and LIBRARIES for libavcodec, libavutil and libswscale.
# Note this is currently not used, since the VPMedia script will grab -I, -L, -l
# etc from vpmedia-config.
#
# Created on: Dec 23, 2010
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
	message("Doing pkg config ffmpeg check...")
	pkg_check_modules(LIBAVCODEC
		libavcodec>=52
		)
	pkg_check_modules(LIBAVUTIL
		libavutil>=50
		)
	pkg_check_modules(LIBSWSCALE
		libswscale
		)
else()
	message("Warning: could not confirm libavcodec version (pkg-config not available).")
	message("Ensure your libavcodec version is >= 52 for a clean build.")
endif()

find_path(
	LIBAVCODEC_INCLUDE_DIRS
	libavcodec/avcodec.h
	HINTS ${LIBAVCODEC_HOME}/include ${LIBAVCODEC_INCLUDE_DIRS}
	)

find_library(
	LIBAVCODEC_LIBRARIES
	avcodec
	HINTS ${LIBAVCODEC_HOME}/lib ${LIBAVCODEC_LIBRARY_DIRS}
	)

find_path(
	LIBAVUTIL_INCLUDE_DIRS
	libavutil/avutil.h
	HINTS ${LIBAVUTIL_HOME}/include ${LIBAVUTIL_INCLUDE_DIRS}
	)

find_library(
	LIBAVUTIL_LIBRARIES
	avutil
	HINTS ${LIBAVUTIL_HOME}/lib ${LIBAVUTIL_LIBRARY_DIRS}
	)

find_path(
	LIBSWSCALE_INCLUDE_DIRS
	libswscale/swscale.h
	HINTS ${LIBSWSCALE_HOME}/include ${LIBSWSCALE_INCLUDE_DIRS}
	)

find_library(
	LIBSWSCALE_LIBRARIES
	swscale
	HINTS ${LIBSWSCALE_HOME}/lib ${LIBSWSCALE_LIBRARY_DIRS}
	)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
	LIBAVCODEC
	DEFAULT_MSG
	LIBAVCODEC_INCLUDE_DIRS
	LIBAVCODEC_LIBRARIES
	)

find_package_handle_standard_args(
	LIBAVUTIL
	DEFAULT_MSG
	LIBAVUTIL_INCLUDE_DIRS
	LIBAVUTIL_LIBRARIES
	)

find_package_handle_standard_args(
	LIBSWSCALE
	DEFAULT_MSG
	LIBSWSCALE_INCLUDE_DIRS
	LIBSWSCALE_LIBRARIES
	)

