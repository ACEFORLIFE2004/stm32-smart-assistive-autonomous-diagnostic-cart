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
#include "storage_controller.h"

#define MAX_AMPLITUDE	FIXEDPT_CONST(1.45f)

//#define MIN_SAMPLE_SIZE	100		/* Arbritrary factor of max sample size */
#define MAX_SAMPLE_SIZE	200	/* Based on best resolution of 12-bit DAC with 3.3V VREF+ and RAM limitations*/

#define ARB_WAV_CHUNK_SAMPLES    4096U
#define WAV_STANDARD_DATA_OFFSET 44U   /* good for standard PCM WAVs with 44-byte header */

#define MIN_WAVE_FREQ	2		/* Based on min conversion frequency which is 5.04kHz and max sample size */
#define MAX_WAVE_FREQ	10000	/* Based on max conversion frequency which is 1MS/s (1MHz) and min sample size */

typedef enum {
    ARB_STREAM_IDLE = 0,
    ARB_STREAM_LOAD_HEADER,
    ARB_STREAM_PRIME_BUF0,
    ARB_STREAM_PRIME_BUF1,
    ARB_STREAM_START_DAC,
    ARB_STREAM_RUNNING,
    ARB_STREAM_DONE,
    ARB_STREAM_ERROR
} ArbStreamState_t;

typedef struct DiagnosticConfig{
	uint8_t wave_gen_mode;		/* This parameter can be a value of @ref wave_gen_mode */

	/* Mainly for use in Function Waveform Generation Mode */
	uint8_t wav_function;		/* This parameter can be a value of @ref wave_func */
	fixedpt wav_amplitude;		/* Must be between 0 - 3.3 */
	uint32_t wav_frequency;		/* Must be between MIN_WAVE_FREQ (2Hz) and MAX_WAVE_FREQ (10000Hz) */

	uint8_t operation;			/* This parameter can be a value of @ref Wave_Operation */
	struct DiagnosticConfig *arg1;
	struct DiagnosticConfig *arg2;

	fixedpt *wave_samples, *dsp_temp_buf; /* Could do: fixedpt *wave_samples = malloc(sizeof(fixedpt)*samples); */


	/* Mainly for use in Arbitrary Waveform Generation Mode */
	char file_name[STORAGE_NAME_LEN];
	File_t *wav_file;
	WAV_Header_t *wav_header;

	FIL stream_fil;
	uint8_t stream_file_open;

	uint32_t file_sample_index;
	uint32_t data_offset;
	uint8_t eof_reached;
	uint8_t loop_enable;

	volatile uint8_t abort_stream;
	volatile uint8_t stream_active;
	volatile uint8_t stream_done;
	volatile uint8_t stream_error;

	ArbStreamState_t stream_state;

//	uint8_t num_channels;		/* This parameter can be a value of @ref Num_Channels */
//	uint32_t sample_rate;
//	uint8_t bytes_per_sample;	/* Block align */
//	uint32_t data_offset, data_size;

	fixedpt *rw_buffer_0, *rw_buffer_1;
	fixedpt *dsp_buf_0, *dsp_buf_1;


	/* Applies to both (all) waveform generation modes */
	uint8_t digital_filter;		/* This parameter can be a value of @ref DSP_Filter */
	uint16_t sample_size;

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

/** @defgroup Num_Channels Can be MONO / STEREO
  * @{
  */
#define MONO			(0x01U)
#define STEREO			(0x02U)

/** @defgroup DSP_Filter Apply FIR / IIR / -- / -- digital filters
  * @{
  */
#define NO_FILTER		(0x00U)
#define ECHO			(0x01U)
#define REVERB			(0x02U)
#define QUADRATURE		(0x03U)

/** @defgroup Screen_Output Display Actual waveform / FFT of waveform on the TFT LCD screen
  * @{
  */
#define	WAVE_DISPLAY	(0x00U)
#define FFT_DISPLAY		(0x01U)
#define	DEFAULT_DISPLAY	WAVE_DISPLAY


void stream_wave_function(DiagnosticConfig_t *func_wave);
void stream_arbitrary_wave(DiagnosticConfig_t *arby_wave);

/* Returns the actual sample size used which depends on user's wave frequency selection */
uint16_t generate_sine_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer);
uint16_t generate_square_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer);
uint16_t generate_triangle_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer);
uint16_t generate_sawtooth_wave(fixedpt amplitude, uint32_t frequency, fixedpt *sample_buffer);

void perform_operation(DiagnosticConfig_t *result);

/* DMA DAC Helper Functions */
void start_dac_conversion(uint32_t conversion_frequency, uint16_t sample_size);
void stop_dac_conversion(void);
uint16_t fxd_to_dhr(fixedpt sample_val);

void start_arbitrary_wave_stream(DiagnosticConfig_t *cfg);
void load_wav_header(File_t *wav_file, WAV_Header_t *wav_header);
//static fixedpt clamp_fxd(fixedpt x, fixedpt lo, fixedpt hi);
//static fixedpt fxd_bipolar_to_dac_volts(fixedpt x);
void arbitrary_wave_mark_buf0_free(void);
void arbitrary_wave_mark_buf1_free(void);
uint16_t *arbitrary_wave_get_dac_buf0(void);
uint16_t *arbitrary_wave_get_dac_buf1(void);
//static StorageStatus_t decode_wav_chunk_inplace_to_fxd(DiagnosticConfig_t *cfg, fixedpt *rw_buffer,
//                                                       uint32_t samples_in_chunk, uint8_t bits_per_sample);
//static void apply_selected_filter_block(DiagnosticConfig_t *cfg, const fixedpt *in, fixedpt *out, uint16_t sample_count);
//static void pack_dac_block_from_fxd(const fixedpt *src, uint16_t *dst, uint16_t sample_count);
//static StorageStatus_t rewind_wav_stream(DiagnosticConfig_t *cfg);

/* TFT LCD Helper Functions */


#endif /* INC_WAVE_GENERATOR_H_ */
