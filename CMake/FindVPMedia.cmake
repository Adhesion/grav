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

#find_library(
#	VPMEDIA_LIBRARIES
#	VPMedia
#	HINTS ${VPMEDIA_HOME}/lib
#	)

#message("vpm ldflags: ${VPMEDIA_LIBRARIES}")

#if(VPMEDIA_FOUND)
message("doing config checks")
find_program(VPMEDIA_CONFIG_EXECUTABLE vpmedia-config)
message("vpm config exec: ${VPMEDIA_CONFIG_EXECUTABLE}")
execute_process(
    COMMAND ${VPMEDIA_CONFIG_EXECUTABLE} --ldflags
    OUTPUT_VARIABLE VPMEDIA_TEMP
    RESULT_VARIABLE RET
    ERROR_QUIET
    )
string(STRIP "${VPMEDIA_TEMP}" VPMEDIA_TEMP)
separate_arguments(VPMEDIA_TEMP)
string(REGEX MATCHALL "-L[^;]+" VPMEDIA_LIBRARY_DIRS "${VPMEDIA_TEMP}")
string(REGEX MATCHALL "-l[^;]+" VPMEDIA_LIBRARIES "${VPMEDIA_TEMP}")
string(REPLACE "-L" "" VPMEDIA_LIBRARY_DIRS "${VPMEDIA_LIBRARY_DIRS}")
string(REPLACE "-l" "" VPMEDIA_LIBRARIES "${VPMEDIA_LIBRARIES}")
set( VPMEDIA_LIBRARY_DIRS "${VPMEDIA_LIBRARY_DIRS}"
	CACHE PATH "VPMedia library directories"
	)
set( VPMEDIA_LIBRARIES "${VPMEDIA_LIBRARIES}"
	CACHE STRING "VPMedia library names"
	)
message("vpm config ldflags output: ${VPMEDIA_TEMP}")
message("vpm lib dirs: ${VPMEDIA_LIBRARY_DIRS}")
message("vpm libs: ${VPMEDIA_LIBRARIES}")
message("retval ${RET}")
#include(CheckSymbolExists)
#check_symbol_exists(VPMEDIA_HAVE_FFMPEG VPMedia/VPMedia_config.h VPMEDIA_HAVE_FFMPEG)
#check_symbol_exists(VPMEDIA_HAVE_LIBDV VPMedia/VPMedia_config.h VPMEDIA_HAVE_LIBDV)
#check_symbol_exists(VPMEDIA_HAVE_THEORA VPMedia/VPMedia_config.h VPMEDIA_HAVE_THEORA)
#message("have ffmpeg is ${VPMEDIA_HAVE_FFMPEG}")
#message("have libdv is ${VPMEDIA_HAVE_LIBDV}")
#message("have theora is ${VPMEDIA_HAVE_THEORA}")
#endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
	VPMEDIA
	DEFAULT_MSG
	VPMEDIA_INCLUDE_DIRS
	VPMEDIA_LIBRARIES
	)

