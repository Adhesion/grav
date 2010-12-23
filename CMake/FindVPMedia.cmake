# @file FindVPMedia.cmake
#
# Find VPMedia libs and include paths.
#
# Created on: Dec 23, 2010
# @author Andrew Ford

find_path(
	VPMEDIA_INCLUDE_DIRS
	VPMedia/VPMedia.h
	HINTS ${VPMEDIA_HOME}/include
	)

find_library(
	VPMEDIA_LIBRARIES
	VPMedia
	HINTS ${VPMEDIA_HOME}/lib
	)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
	VPMEDIA
	DEFAULT_MSG
	VPMEDIA_INCLUDE_DIRS
	VPMEDIA_LIBRARIES
	)

