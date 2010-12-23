# @file FindFFmpeg.cmake
#
# Find FFmpeg libs and include paths.
# Will set INCLUDE_DIRS and LIBRARIES for libavcodec, libavutil and libswscale.
#
# Created on: Dec 23, 2010
# @author Andrew Ford

include(FindPkgConfig)

if(PKG_CONFIG_FOUND)
	message("Doing pkg config ffmpeg check...")
	pkg_check_modules(LIBAVCODEC
		libavcodec>=52
		)
	pkg_check_modules(LIBAVUTIL
		libavutil
		)
	pkg_check_modules(LIBSWSCALE
		libswscale
		)
endif()

if(NOT LIBAVCODEC_FOUND)
	message("Warning: could not confirm libavcodec version (pkg-config not available).")
	message("Ensure your libavcodec version is >= 52 for a clean build.")
	message(${CMAKE_FIND_LIBRARY_SUFFIXES})

	find_path(
		LIBAVCODEC_INCLUDE_DIRS
		libavcodec/avcodec.h
		HINTS ${LIBAVCODEC_HOME}/include
		)

	find_library(
		LIBAVCODEC_LIBRARIES
		avcodec
		HINTS ${LIBAVCODEC_HOME}/lib
		)

	find_path(
		LIBAVUTIL_INCLUDE_DIRS
		libavutil/avutil.h
		HINTS ${LIBAVUTIL_HOME}/include
		)

	find_library(
		LIBAVUTIL_LIBRARIES
		avutil
		HINTS ${LIBAVUTIL_HOME}/lib
		)

	find_path(
		LIBSWSCALE_INCLUDE_DIRS
		libswscale/swscale.h
		HINTS ${LIBSWSCALE_HOME}/include
		)

	find_library(
		LIBSWSCALE_LIBRARIES
		swscale
		HINTS ${LIBSWSCALE_HOME}/lib
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
endif()

