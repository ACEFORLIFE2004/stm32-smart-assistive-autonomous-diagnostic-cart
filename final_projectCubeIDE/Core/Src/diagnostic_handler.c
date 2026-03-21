/*
 * diagnostic_handler.c
 *
 *  Created on: Mar 18, 2026
 *      Author: arnol
 */

#include "diagnostic_handler.h"

/****************************************************************
 *				Imported public System APIs						*
 ****************************************************************/
typedef struct SystemEnv SystemEnv_t;	/* Assist with forward declaration of system's APIs */

SystemEnv_t* get_system_state(void);
void request_diagnostic_state(DiagnosticSubstate_t new_diagnostic_state);

/* Private variables ---------------------------------------------------------*/
static DiagnosticSubstate_t current_diagnostic_state;
static fixedpt sample_buffers[4][MAX_SAMPLE_SIZE];		/* Clear and reuse during different wave gen mode */

uint8_t diag_config_flag;
DiagnosticConfig_t func_wave_1, func_wave_2, func_wave_3, arby_wave;

void initialize_diagnostic_buffers(void){
	/* Each function generated wave gets their own buffer so operations can be performed between them. */
	func_wave_1.wave_samples = sample_buffers;
	func_wave_2.wave_samples = (sample_buffers + MAX_SAMPLE_SIZE);
	func_wave_3.wave_samples = (sample_buffers + (2*MAX_SAMPLE_SIZE));
	/* All function generated waveforms will share the same temporary dsp buffer as DSP filters are applied independently */
	func_wave_1.dsp_temp_buf = (sample_buffers + (3*MAX_SAMPLE_SIZE));
	func_wave_2.dsp_temp_buf = (sample_buffers + (3*MAX_SAMPLE_SIZE));
	func_wave_3.dsp_temp_buf = (sample_buffers + (3*MAX_SAMPLE_SIZE));

	/* R/W Double buffers for DMA2 @ MicroSD and DSP Double buffers DMA1 @ DAC */
	arby_wave.rw_buffer_0	 = sample_buffers;
	arby_wave.rw_buffer_1	 = (sample_buffers + MAX_SAMPLE_SIZE);
	arby_wave.dsp_buf_0		 = (sample_buffers + (2*MAX_SAMPLE_SIZE));
	arby_wave.dsp_buf_1		 = (sample_buffers + (3*MAX_SAMPLE_SIZE));
}

void set_diagnostic_boot_state(void){
	current_diagnostic_state = DIAGNOSTIC_INIT;
}

void diagnostic_update(){
	switch(current_diagnostic_state){
		case DIAGNOSTIC_INIT:
			initialize_diagnostic_buffers();
			diag_config_flag = 0;

			current_diagnostic_state = DIAGNOSTIC_CONFIG;
			request_diagnostic_state(current_diagnostic_state);
			break;
		case DIAGNOSTIC_CONFIG:
			if(diag_config_flag == 1){
				current_diagnostic_state = DIAGNOSTIC_PREPARE;
				request_diagnostic_state(current_diagnostic_state);
			}
			break;
		case DIAGNOSTIC_PREPARE:
			break;
		case DIAGNOSTIC_RUNNING:
			break;
	}
}
