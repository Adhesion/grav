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
find_package(PNG REQUIRED)
find_package(FTGL REQUIRED)
find_package(wxWidgets REQUIRED gl core base)

if(wxWidgets_FOUND)
	include(${wxWidgets_USE_FILE})
endif(wxWidgets_FOUND)

include_directories(
	${OPENGL_INCLUDE_DIR}
	${PNG_INCLUDE_DIR}
	${FREETYPE_INCLUDE_DIRS}
	${FTGL_INCLUDE_DIRS}
	${wxWidgets_INCLUDE_DIRS}
	)

# for ffmpeg
add_definitions(-D__STDC_CONSTANT_MACROS)

set(SOURCES
	AudioManager.cpp
	Earth.cpp
	Frame.cpp
	GLCanvas.cpp
	GLUtil.cpp
	grav.cpp
	gravManager.cpp
	Group.cpp
	InputHandler.cpp
	LayoutManager.cpp
	PNGLoader.cpp
	Point.cpp
	PythonTools.cpp
	RectangleBase.cpp
	Runway.cpp
	SessionManager.cpp
	SessionTreeControl.cpp
	SideFrame.cpp
	Timers.cpp
	TreeControl.cpp
	TreeNode.cpp
	VideoInfoDialog.cpp
	VideoListener.cpp
	VideoSource.cpp
	)

add_executable(grav ${SOURCES})

target_link_libraries(grav
	${OPENGL_LIBRARIES}
	${PNG_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${FTGL_LIBRARIES}
	${wxWidgets_LIBRARIES}
	)

