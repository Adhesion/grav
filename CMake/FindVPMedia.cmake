# @file FindVPMedia.cmake
#
# Find VPMedia libs and include paths.
#
# Created on: Dec 23, 2010
# @author Andrew Ford

find_program(VPMEDIA_CONFIG_EXECUTABLE vpmedia-config)

if(VPMEDIA_CONFIG_EXECUTABLE)
	# grab include dirs from vpmedia cppflags (vpmedia build script only puts dirs
	# here so it's safe for now to only look at -I)

	execute_process(
		COMMAND ${VPMEDIA_CONFIG_EXECUTABLE} --cppflags
		OUTPUT_VARIABLE VPMEDIA_TEMP
		RESULT_VARIABLE RET
		ERROR_QUIET
		)

	string(STRIP "${VPMEDIA_TEMP}" VPMEDIA_TEMP)
	separate_arguments(VPMEDIA_TEMP)

	string(REGEX MATCHALL "-I[^;]+" VPMEDIA_INCLUDE_DIRS "${VPMEDIA_TEMP}")
	string(REPLACE "-I" "" VPMEDIA_INCLUDE_DIRS "${VPMEDIA_INCLUDE_DIRS}")

	set(VPMEDIA_INCLUDE_DIRS "${VPMEDIA_INCLUDE_DIRS}"
		CACHE PATH "VPMedia include directories"
		)

	# now find libs and lib dirs via --ldflags

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

	set(VPMEDIA_LIBRARY_DIRS "${VPMEDIA_LIBRARY_DIRS}"
		CACHE PATH "VPMedia library directories"
		)
	set(VPMEDIA_LIBRARIES "${VPMEDIA_LIBRARIES}"
		CACHE STRING "VPMedia library names"
		)

	include(FindPackageHandleStandardArgs)

	find_package_handle_standard_args(
		VPMEDIA
		DEFAULT_MSG
		VPMEDIA_INCLUDE_DIRS
		VPMEDIA_LIBRARIES
		)
else()
	message("VPMedia config executable not found - unable to include VPMedia")
endif()

