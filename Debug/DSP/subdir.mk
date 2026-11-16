################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../DSP/arm_bitreversal.c \
../DSP/arm_bitreversal2.c \
../DSP/arm_cfft_f32.c \
../DSP/arm_cfft_init_f32.c \
../DSP/arm_cfft_radix8_f32.c \
../DSP/arm_common_tables.c \
../DSP/arm_const_structs.c \
../DSP/arm_rfft_fast_f32.c \
../DSP/arm_rfft_fast_init_f32.c 

OBJS += \
./DSP/arm_bitreversal.o \
./DSP/arm_bitreversal2.o \
./DSP/arm_cfft_f32.o \
./DSP/arm_cfft_init_f32.o \
./DSP/arm_cfft_radix8_f32.o \
./DSP/arm_common_tables.o \
./DSP/arm_const_structs.o \
./DSP/arm_rfft_fast_f32.o \
./DSP/arm_rfft_fast_init_f32.o 

C_DEPS += \
./DSP/arm_bitreversal.d \
./DSP/arm_bitreversal2.d \
./DSP/arm_cfft_f32.d \
./DSP/arm_cfft_init_f32.d \
./DSP/arm_cfft_radix8_f32.d \
./DSP/arm_common_tables.d \
./DSP/arm_const_structs.d \
./DSP/arm_rfft_fast_f32.d \
./DSP/arm_rfft_fast_init_f32.d 


# Each subdirectory must supply rules for building sources it contributes
DSP/%.o DSP/%.su DSP/%.cyclo: ../DSP/%.c DSP/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_NUCLEO_64 -DUSE_HAL_DRIVER -DSTM32L476xx -c -I../Core/Inc -I"C:/Users/viber/Documents/STM32 Stuff/Final Project/DSP/Include" -I"C:/Users/viber/Documents/STM32 Stuff/Final Project/DSP/PrivateInclude" -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/BSP/STM32L4xx_Nucleo -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-DSP

clean-DSP:
	-$(RM) ./DSP/arm_bitreversal.cyclo ./DSP/arm_bitreversal.d ./DSP/arm_bitreversal.o ./DSP/arm_bitreversal.su ./DSP/arm_bitreversal2.cyclo ./DSP/arm_bitreversal2.d ./DSP/arm_bitreversal2.o ./DSP/arm_bitreversal2.su ./DSP/arm_cfft_f32.cyclo ./DSP/arm_cfft_f32.d ./DSP/arm_cfft_f32.o ./DSP/arm_cfft_f32.su ./DSP/arm_cfft_init_f32.cyclo ./DSP/arm_cfft_init_f32.d ./DSP/arm_cfft_init_f32.o ./DSP/arm_cfft_init_f32.su ./DSP/arm_cfft_radix8_f32.cyclo ./DSP/arm_cfft_radix8_f32.d ./DSP/arm_cfft_radix8_f32.o ./DSP/arm_cfft_radix8_f32.su ./DSP/arm_common_tables.cyclo ./DSP/arm_common_tables.d ./DSP/arm_common_tables.o ./DSP/arm_common_tables.su ./DSP/arm_const_structs.cyclo ./DSP/arm_const_structs.d ./DSP/arm_const_structs.o ./DSP/arm_const_structs.su ./DSP/arm_rfft_fast_f32.cyclo ./DSP/arm_rfft_fast_f32.d ./DSP/arm_rfft_fast_f32.o ./DSP/arm_rfft_fast_f32.su ./DSP/arm_rfft_fast_init_f32.cyclo ./DSP/arm_rfft_fast_init_f32.d ./DSP/arm_rfft_fast_init_f32.o ./DSP/arm_rfft_fast_init_f32.su

.PHONY: clean-DSP

