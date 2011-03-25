# @file CMakeLists.txt
#
# CMake build script for grav.
#
# @author Andrew Ford

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

# for ffmpeg
add_definitions(-D__STDC_CONSTANT_MACROS)

# setup paths for install
set(GRAV_RESOURCE_DIR \"${CMAKE_INSTALL_PREFIX}/share/grav/\")
add_definitions(-DGRAV_RESOURCE_DIR=${GRAV_RESOURCE_DIR})

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
	src/gravManager.cpp
	src/gravUtil.cpp
	src/Group.cpp
	src/InputHandler.cpp
	src/LayoutManager.cpp
	src/PNGLoader.cpp
	src/Point.cpp
	src/PythonTools.cpp
	src/RectangleBase.cpp
	src/Runway.cpp
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

