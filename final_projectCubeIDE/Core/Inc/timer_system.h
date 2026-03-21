/*
 * timer_system.h
 *
 *  Created on: Mar 17, 2026
 *      Author: arnol
 */

#ifndef INC_TIMER_SYSTEM_H_
#define INC_TIMER_SYSTEM_H_

#include "config.h"
#include "debugger.h"

#define TIMx_CLK	90000000

typedef uint32_t exectim_t;

extern exectim_t time_elapsed;

/* Private Variables */
//extern char tim_sys_msg_buf[50];

typedef enum{
	MILLI_SECONDS	= 0U,
	MICRO_SECONDS	= 1U,
	NANO_SECONDS	= 2U,
	DEFAULT_RESOLUTION = MICRO_SECONDS
}ExecTime_ResolutionTypeDef;

typedef enum{
	LOW_RANGE		= 0U,		/* Precision of 11ns <--> Range: 0s to 47.72185883s <--> PSC = 0 */
	MEDIUM_RANGE	= 4U,		/* Precision of 55ns <--> Range: 0s to 238.6092942s <--> PSC = 4 */
	HIGH_RANGE		= 8U,		/* Precision of 100ns <--> Range: 0s to 429.4967295s <--> PSC = 8 */
	DEFAULT_RANGE	= LOW_RANGE	/* Same as LOW_RANGE */
}ExecTime_RangeTypeDef;

void measure_exec_time(ExecTime_RangeTypeDef exec_time_range);
exectim_t get_exec_time(char *function_name, ExecTime_RangeTypeDef exec_time_range, ExecTime_ResolutionTypeDef exec_time_resolution);

/* DMA DAC Helper Functions */
void enable_dac_dma_trigger(uint32_t conv_freq);
void disable_dac_dma_trigger(void);

#endif /* INC_TIMER_SYSTEM_H_ */
