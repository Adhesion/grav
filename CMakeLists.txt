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
find_package(FFmpeg REQUIRED)
find_package(Common REQUIRED)
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
	${LIBAVCODEC_INCLUDE_DIRS}
	${LIBAVUTIL_INCLUDE_DIRS}
	${LIBSWSCALE_INCLUDE_DIRS}
	${COMMON_INCLUDE_DIRS}
	${VPMEDIA_INCLUDE_DIRS}
	${PYTHON_INCLUDE_DIRS}
	include/
	)

link_directories(
	${OPENGL_LIBRARY_DIRS}
	${GLEW_LIBRARY_DIRS}
	${PNG_LIBRARY_DIRS}
	${FREETYPE_LIBRARY_DIRS}
	${FTGL_LIBRARY_DIRS}
	${wxWidgets_LIBRARY_DIRS}
	${LIBAVCODEC_LIBRARY_DIRS}
	${LIBAVUTIL_LIBRARY_DIRS}
	${LIBSWSCALE_LIBRARY_DIRS}
	${COMMON_LIBRARY_DIRS}
	${VPMEDIA_LIBRARY_DIRS}
	${PYTHON_LIBRARY_DIRS}
	)

# for ffmpeg
add_definitions(-D__STDC_CONSTANT_MACROS)

set(SOURCES
	src/AudioManager.cpp
	src/Earth.cpp
	src/Frame.cpp
	src/GLCanvas.cpp
	src/GLUtil.cpp
	src/grav.cpp
	src/gravManager.cpp
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
	${COMMON_LIBRARIES}
	${LIBAVCODEC_LIBRARIES}
	${LIBAVUTIL_LIBRARIES}
	${LIBSWSCALE_LIBRARIES}
	${PYTHON_LIBRARIES}
	)

