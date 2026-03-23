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
	uint16_t sample_size = MAX_SAMPLE_SIZE;

	for(uint16_t i = 0; i < sample_size; i++){
		sample_buffer[i] = FXD_MUL( amplitude , FXD_ADD(sine_LUT_fxd[i] , FXD_FROM_FLOAT(1.2f)) );
		dac_dhr_buf[i] 	 = fxd_to_dhr(sample_buffer[i]);
	}

	uint32_t conv_freq = frequency*sample_size;

	start_dac_conversion(conv_freq, sample_size);

	return sample_size;
}

uint16_t generate_square_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer){
	uint16_t sample_size = MAX_SAMPLE_SIZE;

	for(uint16_t i = 0; i < sample_size; i++){
		fixedpt harmonics = FXD_FROM_FLOAT(0.0f);
		uint16_t LUT_index = 0;

		// Compute LUT_index for each harmonic (1, 3, ..., 2n+1) -> scale amplitudes and add to Fourier series sum
		for(uint8_t n = 0; n<=10; n++){
			uint32_t harmonic_index = 0;

			harmonic_index = ((uint32_t)i * ((2*n)+1)) % sample_size;
			LUT_index = (uint16_t)harmonic_index;

			harmonics = FXD_ADD(
				harmonics,
				FXD_DIV(
					sine_LUT_fxd[LUT_index],
					FXD_ADD(FXD_MUL(FXD_FROM_INT(2), FXD_FROM_INT(n)), FXD_FROM_INT(1))
				)
			);
		}

		harmonics = FXD_DIV( FXD_MUL(FXD_FROM_FLOAT(3.39f) , harmonics) , FIXEDPT_PI );

		sample_buffer[i] = FXD_MUL( amplitude , FXD_ADD(harmonics , FXD_FROM_FLOAT(1.2f)) );
		dac_dhr_buf[i] 	 = fxd_to_dhr(sample_buffer[i]);
	}

	uint32_t conv_freq = frequency*sample_size;

	start_dac_conversion(conv_freq, sample_size);

	return sample_size;

}

uint16_t generate_triangle_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer){
	uint16_t sample_size = MAX_SAMPLE_SIZE;

	for(uint16_t i = 0; i < sample_size; i++){
		fixedpt harmonics = FXD_FROM_FLOAT(0.0f);
		uint16_t LUT_index = 0;

		// Compute LUT_index for each harmonic (1, 3, ..., 2n+1) -> scale amplitudes and add to Fourier series sum
		for(uint8_t n = 0; n<=10; n++){
			uint32_t harmonic_index = 0;
			fixedpt term = FXD_FROM_FLOAT(0.0f);

			harmonic_index = ((uint32_t)i * ((2*n)+1)) % sample_size;
			LUT_index = (uint16_t)harmonic_index;

			term = 	FXD_DIV(
							sine_LUT_fxd[LUT_index],
							FXD_MUL(
									FXD_ADD(FXD_MUL(FXD_FROM_INT(2), FXD_FROM_INT(n)), FXD_FROM_INT(1)),
									FXD_ADD(FXD_MUL(FXD_FROM_INT(2), FXD_FROM_INT(n)), FXD_FROM_INT(1))
									)
							);

			if((n % 2) == 1){
				term = FXD_MUL(term, FXD_FROM_INT(-1));
			}

			harmonics = FXD_ADD(harmonics, term);
		}

		harmonics = FXD_DIV( FXD_MUL(FXD_FROM_INT(8) , harmonics) , FXD_MUL(FIXEDPT_PI , FIXEDPT_PI) );

		sample_buffer[i] = FXD_MUL( amplitude , FXD_ADD(harmonics , FXD_FROM_FLOAT(1.2f)) );
		dac_dhr_buf[i] 	 = fxd_to_dhr(sample_buffer[i]);
	}

	uint32_t conv_freq = frequency*sample_size;

	start_dac_conversion(conv_freq, sample_size);

	return sample_size;
}

uint16_t generate_sawtooth_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer){
	uint16_t sample_size = MAX_SAMPLE_SIZE;

	for(uint16_t i = 0; i < sample_size; i++){
		fixedpt harmonics = FXD_FROM_FLOAT(0.0f);
		uint16_t LUT_index = 0;

		// Compute LUT_index for each harmonic (1, 2, ..., n) -> scale amplitudes and add to Fourier series sum
		for(uint8_t n = 1; n<=21; n++){
			uint32_t harmonic_index = 0;
			fixedpt term = FXD_FROM_FLOAT(0.0f);

			harmonic_index = ((uint32_t)i * n) % sample_size;
			LUT_index = (uint16_t)harmonic_index;

			term = 	FXD_DIV( sine_LUT_fxd[LUT_index] , FXD_FROM_INT(n) );

			if((n % 2) == 0){ /* True if n is even */
				term = FXD_MUL(term, FXD_FROM_INT(-1));
			}

			harmonics = FXD_ADD(harmonics, term);
		}

		harmonics = FXD_DIV( FXD_MUL(FXD_FROM_FLOAT(1.77f) , harmonics) , FIXEDPT_PI );

		sample_buffer[i] = FXD_MUL( amplitude , FXD_ADD(harmonics , FXD_FROM_FLOAT(1.2f)) );
		dac_dhr_buf[i] 	 = fxd_to_dhr(sample_buffer[i]);
	}

	uint32_t conv_freq = frequency*sample_size;

	start_dac_conversion(conv_freq, sample_size);

	return sample_size;
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
