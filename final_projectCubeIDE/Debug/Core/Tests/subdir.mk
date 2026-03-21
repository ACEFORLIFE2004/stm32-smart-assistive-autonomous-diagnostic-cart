################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Tests/project_assert.c \
../Core/Tests/test_project.c \
../Core/Tests/ui_interrupt_emulator.c 

OBJS += \
./Core/Tests/project_assert.o \
./Core/Tests/test_project.o \
./Core/Tests/ui_interrupt_emulator.o 

C_DEPS += \
./Core/Tests/project_assert.d \
./Core/Tests/test_project.d \
./Core/Tests/ui_interrupt_emulator.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Tests/%.o Core/Tests/%.su Core/Tests/%.cyclo: ../Core/Tests/%.c Core/Tests/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../FATFS/Target -I../FATFS/App -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FatFs/src -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Tests

clean-Core-2f-Tests:
	-$(RM) ./Core/Tests/project_assert.cyclo ./Core/Tests/project_assert.d ./Core/Tests/project_assert.o ./Core/Tests/project_assert.su ./Core/Tests/test_project.cyclo ./Core/Tests/test_project.d ./Core/Tests/test_project.o ./Core/Tests/test_project.su ./Core/Tests/ui_interrupt_emulator.cyclo ./Core/Tests/ui_interrupt_emulator.d ./Core/Tests/ui_interrupt_emulator.o ./Core/Tests/ui_interrupt_emulator.su

.PHONY: clean-Core-2f-Tests

