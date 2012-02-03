# @file CMakeLists.txt
#
# CMake build script for grav.
#
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

cmake_minimum_required(VERSION 2.6)

project(grav)

set(CMAKE_MODULE_PATH
	"${CMAKE_MODULE_PATH}"
	"${grav_SOURCE_DIR}/CMake/"
	)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(PNG REQUIRED)
find_package(FTGL REQUIRED)
find_package(wxWidgets REQUIRED gl core base)
find_package(VPMedia REQUIRED)
find_package(PythonLibs REQUIRED)

if(wxWidgets_FOUND)
	include(${wxWidgets_USE_FILE})
endif(wxWidgets_FOUND)

include_directories(
	${OPENGL_INCLUDE_DIR}
	${GLEW_INCLUDE_DIRS}
	${PNG_INCLUDE_DIR}
	${FREETYPE_INCLUDE_DIRS}
	${FTGL_INCLUDE_DIRS}
	${wxWidgets_INCLUDE_DIRS}
	${VPMEDIA_INCLUDE_DIRS}
	${PYTHON_INCLUDE_DIRS}
	# stupid compatibility thing for cmake 2.6 :(
	${PYTHON_INCLUDE_PATH}
	include/
	)

link_directories(
	${OPENGL_LIBRARY_DIRS}
	${GLEW_LIBRARY_DIRS}
	${PNG_LIBRARY_DIRS}
	${FREETYPE_LIBRARY_DIRS}
	${FTGL_LIBRARY_DIRS}
	${wxWidgets_LIBRARY_DIRS}
	${VPMEDIA_LIBRARY_DIRS}
	${PYTHON_LIBRARY_DIRS}
	)

# setup paths for install
set(GRAV_RESOURCE_DIR \"${CMAKE_INSTALL_PREFIX}/share/grav/\")
add_definitions(-DGRAV_RESOURCE_DIR=${GRAV_RESOURCE_DIR})

set(GRAV_VERSION_MAJOR \"0\")
set(GRAV_VERSION_MINOR \"1\")
set(GRAV_VERSION_MICRO \"0\")

add_definitions(-DGRAV_VERSION_MAJOR=${GRAV_VERSION_MAJOR})
add_definitions(-DGRAV_VERSION_MINOR=${GRAV_VERSION_MINOR})
add_definitions(-DGRAV_VERSION_MICRO=${GRAV_VERSION_MICRO})

if(CMAKE_BUILD_TYPE STREQUAL "RELWITHDEBINFO" OR CMAKE_BUILD_TYPE STREQUAL "DEBUG")
	add_definitions("-DGRAV_DEBUG_MODE")
endif()

set(SOURCES
	src/AudioManager.cpp
	src/Camera.cpp
	src/Earth.cpp
	src/Frame.cpp
	src/GLCanvas.cpp
	src/GLUtil.cpp
	src/grav.cpp
	src/gravUtil.cpp
	src/Group.cpp
	src/InputHandler.cpp
	src/LayoutManager.cpp
	src/ObjectManager.cpp
	src/PNGLoader.cpp
	src/Point.cpp
	src/PythonTools.cpp
	src/RectangleBase.cpp
	src/Runway.cpp
	src/SessionEntry.cpp
	src/SessionGroup.cpp
	src/SessionGroupButton.cpp
	src/SessionManager.cpp
	src/SessionTreeControl.cpp
	src/SideFrame.cpp
	src/Timers.cpp
	src/TreeControl.cpp
	src/TreeNode.cpp
	src/Vector.cpp
	src/VenueClientController.cpp
	src/VenueNode.cpp
	src/VideoInfoDialog.cpp
	src/VideoListener.cpp
	src/VideoSource.cpp
	)

add_executable(grav ${SOURCES})

target_link_libraries(grav
	${OPENGL_LIBRARIES}
	${GLEW_LIBRARIES}
	${PNG_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${FTGL_LIBRARIES}
	${wxWidgets_LIBRARIES}
	${VPMEDIA_LIBRARIES}
	${PYTHON_LIBRARIES}
	)

install(TARGETS grav
	RUNTIME DESTINATION bin
	)

install(FILES circle.png border.png earth.png FreeSans.ttf
	DESTINATION share/grav
	)

install(DIRECTORY py/
	DESTINATION share/grav/py
	FILES_MATCHING PATTERN "*.py"
	)

