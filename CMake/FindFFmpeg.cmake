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

