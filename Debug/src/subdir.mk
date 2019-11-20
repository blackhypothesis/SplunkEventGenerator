################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ConnectionParamList.cpp \
../src/Event.cpp \
../src/SplunkEventGenerator.cpp 

OBJS += \
./src/ConnectionParamList.o \
./src/Event.o \
./src/SplunkEventGenerator.o 

CPP_DEPS += \
./src/ConnectionParamList.d \
./src/Event.d \
./src/SplunkEventGenerator.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++17 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


