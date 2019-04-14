################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../RPI_Printer_main.c \
../clientPi.c \
../communication.c \
../printer.c 

OBJS += \
./RPI_Printer_main.o \
./clientPi.o \
./communication.o \
./printer.o 

C_DEPS += \
./RPI_Printer_main.d \
./clientPi.d \
./communication.d \
./printer.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


