# @file FindCommon.cmake
#
# Find UCL common libs and include paths.
#
# Created on: Dec 23, 2010
# @author Andrew Ford

find_path(
	COMMON_INCLUDE_DIRS
	common/rtp.h
	HINTS ${COMMON_HOME}/include
	)

find_library(
	COMMON_LIBRARIES
	uclmmbase
	HINTS ${COMMON_HOME}/lib
	)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
	COMMON
	DEFAULT_MSG
	COMMON_INCLUDE_DIRS
	COMMON_LIBRARIES
	)

