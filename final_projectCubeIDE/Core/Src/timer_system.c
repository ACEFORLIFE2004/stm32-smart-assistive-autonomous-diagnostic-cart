/*
 * timer_system.c
 *
 *  Created on: Mar 17, 2026
 *      Author: arnol
 */

#include "timer_system.h"

exectim_t time_elapsed;

//char tim_sys_msg_buf[50];

void measure_exec_time(ExecTime_RangeTypeDef exec_time_range){
	if(htim5.State != HAL_TIM_STATE_READY){
		HAL_TIM_Base_Stop(&htim5);
	}

	__HAL_TIM_SET_COUNTER(&htim5, 0x00000000);

	__HAL_TIM_SET_PRESCALER(&htim5, exec_time_range);

	__HAL_TIM_SET_AUTORELOAD(&htim5, 0xFFFFFFFF);

	if(HAL_TIM_Base_Start(&htim5) != HAL_OK){
		(void)sprintf(error_msg_buf, "measure_exec_time: Failed to Start\r\n");
		print_error_msg();

		Error_Handler();
	}

	return;
}

exectim_t get_exec_time(char *function_name, ExecTime_RangeTypeDef exec_time_range,
						ExecTime_ResolutionTypeDef exec_time_resolution){
	exectim_t exec_cycles = __HAL_TIM_GET_COUNTER(&htim5);

	if(htim5.State != HAL_TIM_STATE_BUSY){
		(void)sprintf(error_msg_buf, "get_exec_time: Call \"measure_exec_time\" first\r\n");
		print_error_msg();

		print_msg("Section Under Test (SUT): \"%s\" could not be measured\r\n", function_name);
		return 0x00000000;		/* Think some more about what to put here */
	}

	uint8_t upscaler = 11;
	if(exec_time_range == MEDIUM_RANGE){
		upscaler = 55;
	}else if(exec_time_range == HIGH_RANGE){
		upscaler = 100;
	}

	switch(exec_time_resolution){
		case MILLI_SECONDS:
			exec_cycles = (exec_cycles*upscaler)/1000000;	/* Yes, there will be truncation */
			print_msg("<+> \"%s\" executes in %lums\r\n", function_name, exec_cycles);
			break;
		case MICRO_SECONDS:
			exec_cycles = (exec_cycles*upscaler)/1000;		/* Yes, there will be truncation */
			print_msg("<+> \"%s\" executes in %luus\r\n", function_name, exec_cycles);
			break;
		case NANO_SECONDS:
			exec_cycles*=upscaler;
			print_msg("<+> \"%s\" executes in %luns\r\n", function_name, exec_cycles);
			break;
		default:
			exec_cycles = (exec_cycles*upscaler)/1000;		/* Yes, there will be truncation */
			print_msg("<+> \"%s\" executes in %luus\r\n", function_name, exec_cycles);
	}

	HAL_TIM_Base_Stop(&htim5);

	return exec_cycles;
}

/* DMA DAC Helper Functions */
void enable_dac_dma_trigger(uint32_t conv_freq){
	if(htim6.State != HAL_TIM_STATE_READY){
		HAL_TIM_Base_Stop(&htim6);
	}

	uint16_t PSC = 0;
	uint32_t ARR = (TIMx_CLK / (conv_freq)) - 1;

	while(ARR > 0xFFFF){
		PSC++;
		ARR = (TIMx_CLK / ((conv_freq)*(PSC+1))) - 1;
	}

	__HAL_TIM_SET_COUNTER(&htim6, 0x00000000);

	__HAL_TIM_SET_PRESCALER(&htim6, PSC);

	__HAL_TIM_SET_AUTORELOAD(&htim6, (uint16_t)ARR);

	if(HAL_TIM_Base_Start(&htim6) != HAL_OK){
		(void)sprintf(error_msg_buf, "enable_dac_dma_trigger: Failed to enable TRGO_T6 trigger\r\n");
		print_error_msg();

		Error_Handler();
	}
}

void disable_dac_dma_trigger(void){
	if(htim6.State != HAL_TIM_STATE_BUSY){
		(void)sprintf(error_msg_buf, "disable_dac_dma_trigger: Call \"enable_dac_dma_trigger\" first\r\n");
		print_error_msg();

		print_msg("Could not disable TRGO_T6 trigger because trigger not enabled\r\n");
		return;
	}

	HAL_TIM_Base_Stop(&htim6);
}

void motor_set_duty(uint8_t duty, MotorType_t motor, MotorDir_t direction){
	uint16_t period = __HAL_TIM_GET_AUTORELOAD(&htim2);

	uint16_t CCR = (duty*period)/100;

	switch(motor){
		case LEFT_MOTOR:
			if((direction == FORWARD_DIR) ||  (direction == NO_DIR)){
				__HAL_TIM_SET_COMPARE(&htim11, TIM_CHANNEL_1, CCR);	/* Drive IN1 w/ PWM */
				__HAL_TIM_SET_COMPARE(&htim14, TIM_CHANNEL_1, 0);	/* Drive IN2 low */
			}else if(direction == REVERSE_DIR){
				__HAL_TIM_SET_COMPARE(&htim11, TIM_CHANNEL_1, 0);	/* Drive IN1 low */
				__HAL_TIM_SET_COMPARE(&htim14, TIM_CHANNEL_1, CCR);	/* Drive IN2 w/ PWM */
			}
			break;
		case RIGHT_MOTOR:
			if(direction == FORWARD_DIR || direction == NO_DIR){
				__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, CCR);	/* Drive IN1 w/ PWM */
				__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, 0);	/* Drive IN2 low */
			}else if(direction == REVERSE_DIR){
				__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0);	/* Drive IN1 low */
				__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, CCR);	/* Drive IN2 w/ PWM */
			}
			break;
	}
}

void motor_set_frequency(uint16_t frequency){
	uint16_t PSC = 0;
	uint32_t ARR = (TIMx_CLK / (frequency)) - 1;

	while(ARR > 0xFFFF){
		PSC++;
		ARR = (TIMx_CLK / ((frequency)*(PSC+1))) - 1;
	}

	__HAL_TIM_SET_PRESCALER(&htim2, PSC);

	__HAL_TIM_SET_PRESCALER(&htim11, PSC);
	__HAL_TIM_SET_PRESCALER(&htim14, PSC);

	__HAL_TIM_SET_AUTORELOAD(&htim2, (uint16_t)ARR);
	__HAL_TIM_SET_AUTORELOAD(&htim11, (uint16_t)ARR);
	__HAL_TIM_SET_AUTORELOAD(&htim14, (uint16_t)ARR);
}

void stop_motor(MotorType_t motor){
	uint16_t period = __HAL_TIM_GET_AUTORELOAD(&htim2);

	uint16_t CCR = period;

	switch(motor){
		case LEFT_MOTOR:
			__HAL_TIM_SET_COMPARE(&htim11, TIM_CHANNEL_1, CCR);	/* Drive IN1 w/ PWM */
			__HAL_TIM_SET_COMPARE(&htim14, TIM_CHANNEL_1, CCR);	/* Drive IN2 low */
			break;
		case RIGHT_MOTOR:
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, CCR);	/* Drive IN1 w/ PWM */
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, CCR);	/* Drive IN2 low */
			break;
	}
}
