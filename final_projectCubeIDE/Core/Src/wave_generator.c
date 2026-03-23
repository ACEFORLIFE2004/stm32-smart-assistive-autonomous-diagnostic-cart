/*
 * wave_generator.c
 *
 *  Created on: Mar 21, 2026
 *      Author: arnol
 */

#include "wave_generator.h"
#include <math.h>

static uint16_t dac_dhr_buf[MAX_SAMPLE_SIZE];	/* The buffer that actually holds the 12-bit dhr values */
static float diag_debug_buf[MAX_SAMPLE_SIZE];	/* Floating point buffer for debugging purposes */

void stream_wave_function(DiagnosticConfig_t *func_wave){
	switch(func_wave->wav_function){
		case SINUSOIDAL_FUNC:
			func_wave->sample_size = generate_sine_wave(func_wave->wav_amplitude, func_wave->wav_frequency, func_wave->wave_samples);
			break;
		case SQUARE_FUNC:
			func_wave->sample_size = generate_square_wave(func_wave->wav_amplitude, func_wave->wav_frequency, func_wave->wave_samples);
			break;
		case TRIANGLE_FUNC:
			func_wave->sample_size = generate_triangle_wave(func_wave->wav_amplitude, func_wave->wav_frequency, func_wave->wave_samples);
			break;
		case SAWTOOTH_FUNC:
			func_wave->sample_size = generate_sawtooth_wave(func_wave->wav_amplitude, func_wave->wav_frequency, func_wave->wave_samples);
			break;
	}
}

void stream_arbitrary_wave(DiagnosticConfig_t *arby_wave){
	return;
}

uint16_t generate_sine_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer){
	uint16_t sample_size = (frequency > 5000) ? MIN_SAMPLE_SIZE : MAX_SAMPLE_SIZE;

	if(sample_size == MAX_SAMPLE_SIZE){
		for(uint16_t i = 0; i < sample_size; i++){
			sample_buffer[i] = FXD_MUL( amplitude , FXD_ADD(sine_LUT_fxd[i] , FXD_FROM_FLOAT(1.2f)) );
			dac_dhr_buf[i] 	 = fxd_to_dhr(sample_buffer[i]);

			/* Floating point sine wave generation for debugging purposes */
//			diag_debug_buf[i] = 1.0f * (sin((i/200.0f)*M_TWOPI) + 1.0f);
//			dac_dhr_buf[i] 	 = (uint16_t)((diag_debug_buf[i] * 4095.0f)/(2.0f * 1.65f));

//			if(i%24 == 0){
//				print_msg("\r\n %u,", dac_dhr_buf[i]);
//			}else{
//				print_msg(" %u,", dac_dhr_buf[i]);
//			}
		}
//		print_msg("\r\n");
	}else{
		for(uint16_t i = 0, y = 0; (i < MAX_SAMPLE_SIZE) && (y < sample_size); i+=(MAX_SAMPLE_SIZE/MIN_SAMPLE_SIZE), y++){
			sample_buffer[y] = FXD_MUL( amplitude , FXD_ADD(sine_LUT_fxd[i] , FXD_FROM_INT(1)) );
			dac_dhr_buf[y] = fxd_to_dhr(sample_buffer[y]);
		}
	}

	uint32_t conv_freq = frequency*sample_size;

	start_dac_conversion(conv_freq, sample_size);

	return sample_size;

}

uint16_t generate_square_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer){
	return 0;
}
uint16_t generate_triangle_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer){
	return 0;
}
uint16_t generate_sawtooth_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer){
	return 0;
}

void perform_operation(DiagnosticConfig_t *result){
//	DiagnosticConfig_t *arg_1 = result->arg1;
//	DiagnosticConfig_t *arg_2 = result->arg2;
//	uint8_t operation 		  = result->operation;
//	fixedpt *result_arg 	  = result->wave_samples;

	return;
}

/* DMA DAC Helper Functions */
void start_dac_conversion(uint32_t conversion_frequency, uint16_t sample_size){
	if(HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)dac_dhr_buf, sample_size, DAC_ALIGN_12B_R) != HAL_OK){
		(void)sprintf(error_msg_buf, "  <err> start_dac_conversion: Failed to start DMA for DAC conversion\r\n");
		print_error_msg();

		while(1){
			;
		}
	}

	enable_dac_dma_trigger(conversion_frequency);
	print_msg("  --> DMA DAC Conversion started\r\n");
}

void stop_dac_conversion(void){
	disable_dac_dma_trigger();

	(void)HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
}

uint16_t fxd_to_dhr(fixedpt sample_val){
	return FXD_TO_INT( FXD_DIV(FXD_MUL(sample_val , FXD_FROM_INT(4095)) , FXD_MUL(FXD_FROM_INT(2) , FXD_FROM_FLOAT(1.65f))) );
}
