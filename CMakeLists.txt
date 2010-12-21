cmake_minimum_required(VERSION 2.6)
project(grav)

find_package(OpenGL REQUIRED)
find_package(PNG REQUIRED)

include_directories(
	${OPENGL_INCLUDE_DIR}
	${PNG_INCLUDE_DIR}
	)

set(SOURCES
	AudioManager.cpp
	Earth.cpp
	GLCanvas.cpp
	GLUtil.cpp
	gravManager.cpp
	grav.cpp
	Group.cpp
	InputHandler.cpp
	LayoutManager.cpp
	PNGLoader.cpp
	RectangleBase.cpp
	TreeControl.cpp
	TreeNode.cpp
	VideoListener.cpp
	VideoSource.cpp
	)

add_executable(grav ${SOURCES})
