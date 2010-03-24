################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../AudioManager.cpp \
../Earth.cpp \
../GLCanvas.cpp \
../GLUtil.cpp \
../Group.cpp \
../InputHandler.cpp \
../LayoutManager.cpp \
../PNGLoader.cpp \
../RectangleBase.cpp \
../TreeControl.cpp \
../TreeNode.cpp \
../VideoListener.cpp \
../VideoSource.cpp \
../grav.cpp \
../gravManager.cpp 

OBJS += \
./AudioManager.o \
./Earth.o \
./GLCanvas.o \
./GLUtil.o \
./Group.o \
./InputHandler.o \
./LayoutManager.o \
./PNGLoader.o \
./RectangleBase.o \
./TreeControl.o \
./TreeNode.o \
./VideoListener.o \
./VideoSource.o \
./grav.o \
./gravManager.o 

CPP_DEPS += \
./AudioManager.d \
./Earth.d \
./GLCanvas.d \
./GLUtil.d \
./Group.d \
./InputHandler.d \
./LayoutManager.d \
./PNGLoader.d \
./RectangleBase.d \
./TreeControl.d \
./TreeNode.d \
./VideoListener.d \
./VideoSource.d \
./grav.d \
./gravManager.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O2 -g3 -Wall -c -fmessage-length=0 `freetype-config --cflags` `vpmedia-config --cppflags` `wx-config --cppflags` -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


