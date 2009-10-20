################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Earth.cpp \
../Group.cpp \
../PNGLoader.cpp \
../RectangleBase.cpp \
../VideoSource.cpp \
../glutVideo.cpp 

OBJS += \
./Earth.o \
./Group.o \
./PNGLoader.o \
./RectangleBase.o \
./VideoSource.o \
./glutVideo.o 

CPP_DEPS += \
./Earth.d \
./Group.d \
./PNGLoader.d \
./RectangleBase.d \
./VideoSource.d \
./glutVideo.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -g3 -Wall -c -fmessage-length=0 `freetype-config --cflags` `vpmedia-config --cppflags` -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


