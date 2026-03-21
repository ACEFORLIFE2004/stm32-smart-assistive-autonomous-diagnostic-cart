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

	__HAL_TIM_SET_COUNTER(&htim6, 0x00000000);

	__HAL_TIM_SET_PRESCALER(&htim6, exec_time_range);

	__HAL_TIM_SET_AUTORELOAD(&htim6, 0xFFFFFFFF);

	if(HAL_TIM_Base_Start(&htim6) != HAL_OK){
		(void)sprintf(error_msg_buf, "enable_dac_dma_trigger: Failed to enable TRGO_T6 trigger\r\n");
		print_error_msg();

		Error_Handler();
	}
}
void disable_dac_dma_trigger(void);
