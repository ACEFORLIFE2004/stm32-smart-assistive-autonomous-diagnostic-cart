/*
 * wave_generator.h
 *
 *  Created on: Mar 21, 2026
 *      Author: arnol
 */

#ifndef INC_WAVE_GENERATOR_H_
#define INC_WAVE_GENERATOR_H_

#include "project.h"
#include "sine_lut.h"
#include "dsp.h"

#define MIN_SAMPLE_SIZE	720		/* Based on max frequency */
#define MAX_SAMPLE_SIZE	2520	/* Based on best resolution of 12-bit DAC with 3.3V VREF+ */

#define MIN_WAVE_FREQ	FIXEDPT_CONST(0)	/* Based on max sample size */
#define MAX_WAVE_FREQ	FIXEDPT_CONST(0xffffffff)

typedef struct DiagnosticConfig{
	uint8_t wave_gen_mode;		/* This parameter can be a value of @ref wave_gen_mode */

	/* Mainly for use in Function Waveform Generation Mode */
	uint8_t wave_func;			/* This parameter can be a value of @ref wave_func */
	fixedpt wav_amplitude;		/* Must be between 0 - 3.3 */
	uint32_t wav_frequency;

	uint8_t operation;			/* This parameter can be a value of @ref Wave_Operation */
	struct DiagnosticConfig *arg1;
	struct DiagnosticConfig *arg2;

	fixedpt *wave_samples, *dsp_temp_buf; /* Could do: fixedpt *wave_samples = malloc(sizeof(fixedpt)*samples); */


	/* Mainly for use in Arbitrary Waveform Generation Mode */
	fixedpt *rw_buffer_0, *rw_buffer_1;
	fixedpt *dsp_buf_0, *dsp_buf_1;


	/* Applies to both (all) waveform generation modes */
	uint8_t digital_filter;		/* This parameter can be a value of @ref DSP_Filter */

	uint8_t screen_output;		/* This parameter can be a value of @ref Screen_Output */
}DiagnosticConfig_t;


/** @defgroup wave_gen_mode Generate waveform from integral function / Generate waveform from external storage device
  * @{
  */
#define FUNCTION_WAVE_GEN_MODE	(0x00U)
#define ARBITRARY_WAVE_GEN_MODE	(0x01U)

/** @defgroup wave_func Generate sinusoidal / square / triangle / sawtooth periodic waveforms
  * @{
  */
#define SINUSOIDAL_FUNC		(0x00U)
#define SQUARE_FUNC			(0x01U)
#define TRIANGLE_FUNC		(0x02U)
#define SAWTOOTH_FUNC		(0x03U)

/** @defgroup Wave_Operation Perform addition / subtraction / multiplication / division between two waves
  * @{
  */
#define OP_NONE			(0x00U)
#define OP_ADD			(0x01U)
#define OP_SUB			(0x02U)
#define OP_MUL			(0x03U)
#define OP_DIV			(0x04U)

/** @defgroup DSP_Filter Apply FIR / IIR / -- / -- digital filters
  * @{
  */
#define NO_FILTER		(0x00U)
#define FIR_FILTER		(0x01U)
#define IIR_FILTER		(0x02U)

/** @defgroup Screen_Output Display Actual waveform / FFT of waveform on the TFT LCD screen
  * @{
  */
#define	WAVE_DISPLAY	(0x00U)
#define FFT_DISPLAY		(0x01U)
#define	DEFAULT_DISPLAY	(0x00U)


void generate_wave_function(DiagnosticConfig_t *func_wave);
void stream_arbitrary_wave(DiagnosticConfig_t *arby_wave);

void generate_sine_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer);
void generate_square_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer);
void generate_triangle_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer);
void generate_sawtooth_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer);

void perform_operation(DiagnosticConfig_t *arg_1, DiagnosticConfig_t *arg_2, uint8_t operation, fixedpt *result_arg);

/* DMA DAC Helper Functions */
void start_dac_conversion(uint32_t conversion_frequency);
void stop_dac_conversion(void);

/* TFT LCD Helper Functions */


#endif /* INC_WAVE_GENERATOR_H_ */
