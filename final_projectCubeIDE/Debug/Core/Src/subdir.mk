################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/assistive_cart_logic.c \
../Core/Src/bluetooth.c \
../Core/Src/config.c \
../Core/Src/debugger.c \
../Core/Src/diagnostic_handler.c \
../Core/Src/fonts.c \
../Core/Src/keypad.c \
../Core/Src/main.c \
../Core/Src/motor_driver.c \
../Core/Src/sine_lut.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system.c \
../Core/Src/system_stm32f4xx.c \
../Core/Src/tft_lcd.c \
../Core/Src/timer_system.c \
../Core/Src/ui_handler.c \
../Core/Src/wave_generator.c 

OBJS += \
./Core/Src/assistive_cart_logic.o \
./Core/Src/bluetooth.o \
./Core/Src/config.o \
./Core/Src/debugger.o \
./Core/Src/diagnostic_handler.o \
./Core/Src/fonts.o \
./Core/Src/keypad.o \
./Core/Src/main.o \
./Core/Src/motor_driver.o \
./Core/Src/sine_lut.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system.o \
./Core/Src/system_stm32f4xx.o \
./Core/Src/tft_lcd.o \
./Core/Src/timer_system.o \
./Core/Src/ui_handler.o \
./Core/Src/wave_generator.o 

C_DEPS += \
./Core/Src/assistive_cart_logic.d \
./Core/Src/bluetooth.d \
./Core/Src/config.d \
./Core/Src/debugger.d \
./Core/Src/diagnostic_handler.d \
./Core/Src/fonts.d \
./Core/Src/keypad.d \
./Core/Src/main.d \
./Core/Src/motor_driver.d \
./Core/Src/sine_lut.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system.d \
./Core/Src/system_stm32f4xx.d \
./Core/Src/tft_lcd.d \
./Core/Src/timer_system.d \
./Core/Src/ui_handler.d \
./Core/Src/wave_generator.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../FATFS/Target -I../FATFS/App -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FatFs/src -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/assistive_cart_logic.cyclo ./Core/Src/assistive_cart_logic.d ./Core/Src/assistive_cart_logic.o ./Core/Src/assistive_cart_logic.su ./Core/Src/bluetooth.cyclo ./Core/Src/bluetooth.d ./Core/Src/bluetooth.o ./Core/Src/bluetooth.su ./Core/Src/config.cyclo ./Core/Src/config.d ./Core/Src/config.o ./Core/Src/config.su ./Core/Src/debugger.cyclo ./Core/Src/debugger.d ./Core/Src/debugger.o ./Core/Src/debugger.su ./Core/Src/diagnostic_handler.cyclo ./Core/Src/diagnostic_handler.d ./Core/Src/diagnostic_handler.o ./Core/Src/diagnostic_handler.su ./Core/Src/fonts.cyclo ./Core/Src/fonts.d ./Core/Src/fonts.o ./Core/Src/fonts.su ./Core/Src/keypad.cyclo ./Core/Src/keypad.d ./Core/Src/keypad.o ./Core/Src/keypad.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/motor_driver.cyclo ./Core/Src/motor_driver.d ./Core/Src/motor_driver.o ./Core/Src/motor_driver.su ./Core/Src/sine_lut.cyclo ./Core/Src/sine_lut.d ./Core/Src/sine_lut.o ./Core/Src/sine_lut.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system.cyclo ./Core/Src/system.d ./Core/Src/system.o ./Core/Src/system.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su ./Core/Src/tft_lcd.cyclo ./Core/Src/tft_lcd.d ./Core/Src/tft_lcd.o ./Core/Src/tft_lcd.su ./Core/Src/timer_system.cyclo ./Core/Src/timer_system.d ./Core/Src/timer_system.o ./Core/Src/timer_system.su ./Core/Src/ui_handler.cyclo ./Core/Src/ui_handler.d ./Core/Src/ui_handler.o ./Core/Src/ui_handler.su ./Core/Src/wave_generator.cyclo ./Core/Src/wave_generator.d ./Core/Src/wave_generator.o ./Core/Src/wave_generator.su

.PHONY: clean-Core-2f-Src

