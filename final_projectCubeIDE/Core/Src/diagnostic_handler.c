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
static fixedpt sample_buffers[4][ARB_WAV_CHUNK_SAMPLES];		/* Clear and reuse during different wave gen mode */
static WAV_Header_t arby_wav_header;

uint8_t func_wav_no;
uint8_t diag_config_flag, diag_running_flag;
DiagnosticConfig_t func_wave_1, func_wave_2, func_wave_3, *arby_wave;

void initialize_diagnostic_buffers(DiagnosticConfig_t *cfg, uint8_t slot){
    // slot 0, 1, or 2 based on which func_wave we are setup-ing
    cfg->wave_samples = sample_buffers[slot];
    cfg->dsp_temp_buf = sample_buffers[3];

    // For Arbitrary mode:
    cfg->rw_buffer_0  = sample_buffers[0];
    cfg->rw_buffer_1  = sample_buffers[1];
    cfg->dsp_buf_0    = sample_buffers[2];
    cfg->dsp_buf_1    = sample_buffers[3];

    // CRITICAL: Link the header pointer to the actual static storage
    cfg->wav_header   = &arby_wav_header;
}

//void initialize_diagnostic_buffers(void){
//	/* Each function generated wave gets their own buffer so operations can be performed between them. */
//	func_wave_1.wave_samples = sample_buffers[0];
//	func_wave_2.wave_samples = sample_buffers[1];
//	func_wave_3.wave_samples = sample_buffers[2];
//	/* All function generated waveforms will share the same temporary dsp buffer as DSP filters are applied independently */
//	func_wave_1.dsp_temp_buf = sample_buffers[3];
//	func_wave_2.dsp_temp_buf = sample_buffers[3];
//	func_wave_3.dsp_temp_buf = sample_buffers[3];
//
//	/* R/W Double buffers for DMA2 @ MicroSD and DSP Double buffers DMA1 @ DAC */
//	arby_wave->rw_buffer_0	 = sample_buffers[0];
//	arby_wave->rw_buffer_1	 = sample_buffers[1];
//	arby_wave->dsp_buf_0	 = sample_buffers[2];
//	arby_wave->dsp_buf_1	 = sample_buffers[3];
//
//	arby_wave->wav_header 	 = &arby_wav_header;
//}

void reset_diagnostic_state(void){
	current_diagnostic_state = DIAGNOSTIC_INIT;
}

void diagnostic_update(){
	switch(current_diagnostic_state){
		case DIAGNOSTIC_INIT:
			print_msg("  --> Entering DIAGNOSTIC_INIT\r\n");

			arby_wave = &func_wave_1;

			initialize_diagnostic_buffers(&func_wave_1, 0);

			diag_config_flag 	= 0;
			diag_running_flag 	= 0;
			func_wav_no 		= 1;

			arby_wave->stream_active = 0;
			arby_wave->stream_done = 0;
			arby_wave->stream_error = 0;
			arby_wave->stream_state = ARB_STREAM_IDLE;

			current_diagnostic_state = DIAGNOSTIC_CONFIG;
			request_diagnostic_state(current_diagnostic_state);
			print_msg("  --> Leaving DIAGNOSTIC_INIT\r\n");
			break;
		case DIAGNOSTIC_CONFIG:
//			print_msg("  --> Entering DIAGNOSTIC_CONFIG\r\n");
			if(diag_config_flag == 1){
				current_diagnostic_state = DIAGNOSTIC_PREPARE;
				request_diagnostic_state(current_diagnostic_state);
				print_msg("  --> Leaving DIAGNOSTIC_CONFIG\r\n");
			}
			break;
		case DIAGNOSTIC_PREPARE:
			print_msg("  --> Entering DIAGNOSTIC_PREPARE\r\n");
			switch(func_wav_no){
				case 1:
					if(func_wave_1.operation == OP_NONE){
						if(func_wave_1.wave_gen_mode == FUNCTION_WAVE_GEN_MODE){
							stream_wave_function(&func_wave_1);
							print_msg("Plotting Wave 1\r\n");
						}else if(func_wave_1.wave_gen_mode == ARBITRARY_WAVE_GEN_MODE){

							if(arby_wave->stream_active == 0){
								arby_wave = &func_wave_1;

	//							initialize_diagnostic_buffers();

								arby_wave->stream_active = 1;

								arby_wave->sample_size = ARB_WAV_CHUNK_SAMPLES;
							}

							stream_arbitrary_wave(arby_wave);

							if(arby_wave->stream_state != ARB_STREAM_RUNNING){
								current_diagnostic_state = DIAGNOSTIC_PREPARE;
								request_diagnostic_state(current_diagnostic_state);
								return;
							}
						}
					}else{
						func_wave_1.arg1 = &func_wave_2;
						func_wave_1.arg2 = &func_wave_3;
						perform_operation(&func_wave_1);
					}
					break;
				case 2:
					if(func_wave_2.operation == OP_NONE){
						if(func_wave_2.wave_gen_mode == FUNCTION_WAVE_GEN_MODE){
							stream_wave_function(&func_wave_2);
							print_msg("Plotting Wave 2\r\n");
						}else if(func_wave_2.wave_gen_mode == ARBITRARY_WAVE_GEN_MODE){
							arby_wave = &func_wave_2;

//							initialize_diagnostic_buffers();

							arby_wave->stream_active = 1;

							stream_arbitrary_wave(arby_wave);

							if(arby_wave->stream_state != ARB_STREAM_RUNNING){
								current_diagnostic_state = DIAGNOSTIC_PREPARE;
								request_diagnostic_state(current_diagnostic_state);
								return;
							}
						}
					}else{
						func_wave_2.arg1 = &func_wave_3;
						func_wave_2.arg2 = &func_wave_1;
						perform_operation(&func_wave_2);
					}
					break;
				case 3:
					if(func_wave_3.operation == OP_NONE){
						if(func_wave_3.wave_gen_mode == FUNCTION_WAVE_GEN_MODE){
							stream_wave_function(&func_wave_3);
							print_msg("Plotting Wave 3\r\n");
						}else if(func_wave_3.wave_gen_mode == ARBITRARY_WAVE_GEN_MODE){
							arby_wave = &func_wave_3;

//							initialize_diagnostic_buffers();

							arby_wave->stream_active = 1;

							stream_arbitrary_wave(arby_wave);

							if(arby_wave->stream_state != ARB_STREAM_RUNNING){
								current_diagnostic_state = DIAGNOSTIC_PREPARE;
								request_diagnostic_state(current_diagnostic_state);
								return;
							}
						}
					}else{
						func_wave_3.arg1 = &func_wave_1;
						func_wave_3.arg2 = &func_wave_2;
						perform_operation(&func_wave_3);
					}
					break;
			}

			current_diagnostic_state = DIAGNOSTIC_RUNNING;
			request_diagnostic_state(current_diagnostic_state);
			print_msg("  --> Leaving DIAGNOSTIC_PREPARE\r\n");
			break;
		case DIAGNOSTIC_RUNNING:
			if(diag_config_flag != 1){	/* Preferably UI sets config_flag to 0 while in diag_running state */
				if ((arby_wave != NULL) && (arby_wave->wave_gen_mode == ARBITRARY_WAVE_GEN_MODE)) {
					arby_wave->abort_stream = 1U;
					stream_arbitrary_wave(arby_wave);
				}

				/* User has requested to configure a different waveform */
				func_wav_no = (func_wav_no % 3) + 1;
				current_diagnostic_state = DIAGNOSTIC_CONFIG;
				request_diagnostic_state(current_diagnostic_state);

				stop_dac_conversion();
			}else{
				if(arby_wave->wave_gen_mode == ARBITRARY_WAVE_GEN_MODE){
					stream_arbitrary_wave(arby_wave);
				}

				diag_running_flag = 1;	/* Every iteration, UI should set this to 0 after drawing on screen */
			}
			break;
	}
}
