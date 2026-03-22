/*
 * wave_generator.c
 *
 *  Created on: Mar 21, 2026
 *      Author: arnol
 */

#include "wave_generator.h"

static uint16_t dac_dhr_buf[MAX_SAMPLE_SIZE];	/* The buffer that actually holds the 12-bit dhr values */

void generate_wave_function(DiagnosticConfig_t *func_wave){
	switch(func_wave->wave_func){
		case SINUSOIDAL_FUNC:
			generate_sine_wave(func_wave->wav_amplitude, func_wave->wav_frequency, func_wave->wave_samples);
			break;
		case SQUARE_FUNC:
			generate_square_wave(func_wave->wav_amplitude, func_wave->wav_frequency, func_wave->wave_samples);
			break;
		case TRIANGLE_FUNC:
			generate_triangle_wave(func_wave->wav_amplitude, func_wave->wav_frequency, func_wave->wave_samples);
			break;
		case SAWTOOTH_FUNC:
			generate_sawtooth_wave(func_wave->wav_amplitude, func_wave->wav_frequency, func_wave->wave_samples);
			break;
	}
}

void stream_arbitrary_wave(DiagnosticConfig_t *arby_wave);

void generate_sine_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer);
void generate_square_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer);
void generate_triangle_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer);
void generate_sawtooth_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer);

void perform_operation(DiagnosticConfig_t *arg_1, DiagnosticConfig_t *arg_2, uint8_t operation, fixedpt *result_arg);

/* DMA DAC Helper Functions */
void start_dac_conversion(uint32_t conversion_frequency, uint16_t sample_size){
	if(HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, dac_dhr_buf, sample_size, DAC_ALIGN_12B_R) != HAL_OK){
		(void)sprintf(error_msg_buf, "  <err> start_dac_conversion: Failed to start DMA for DAC conversion\r\n");
		print_error_msg();

		while(1){
			;
		}
	}

	enable_dac_dma_trigger(conversion_frequency);
}

void stop_dac_conversion(void){
	disable_dac_dma_trigger();

	(void)HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
}

uint16_t fxd_to_dhr(fixedpt sample_val){
	return FXD_DIV(FXD_MUL(sample_val,4095),(FXD_MUL(2,MAX_AMPLITUDE)));
}
